# Архитектура: Hotels Booking System (v2)

Схема компонентов: [images/uml.drawio](../images/uml.drawio).

Внешний API Gateway (`:8080`) описан в [OpenAPI](../v2/%5Binst%5D%5Bv2%5D%20Hotels%20Booking%20System.yml).
Здесь описаны внутренние контракты сервисов и то, как Gateway их оркестрирует.
Сервисы друг друга не вызывают — только Gateway вызывает сервисы.

Во всех сервисах есть `GET /manage/health` → `200`.

## Внутренние API

### Reservation Service (`:8070`)

| Метод | Путь | Заголовок | Тело запроса | Ответ |
|---|---|---|---|---|
| GET | `/api/v1/hotels?page=&size=` | — | — | `200` `PaginationResponse` (как во внешнем API) |
| GET | `/api/v1/hotels/{hotelUid}` | — | — | `200` `HotelResponse` (с `price`) / `404` |
| GET | `/api/v1/reservations` | `X-User-Name` | — | `200` `[Reservation]` |
| GET | `/api/v1/reservations/{reservationUid}` | `X-User-Name` | — | `200` `Reservation` / `404` |
| POST | `/api/v1/reservations` | `X-User-Name` | `{hotelUid, startDate, endDate, paymentUid}` | `201` `Reservation` / `404` (нет отеля) |
| DELETE | `/api/v1/reservations/{reservationUid}` | `X-User-Name` | — | `204` / `404` |

```json
// Reservation
{
  "reservationUid": "…",
  "hotel": { "hotelUid": "…", "name": "…", "fullAddress": "Россия, Москва, Неглинная ул., 4", "stars": 5 },
  "startDate": "2021-10-08",
  "endDate": "2021-10-11",
  "status": "PAID",
  "paymentUid": "…"
}
```

- `hotel` уже в форме `HotelInfo` из внешнего API: `fullAddress` собирает Reservation (данные у него), чтение — JOIN `reservation` с `hotels`.
- Бронь ищется по `reservation_uid AND username`, поэтому чужая бронь неотличима от несуществующей → `404`.
- `DELETE` переводит статус в `CANCELED`, запись не удаляется.

### Payment Service (`:8060`)

| Метод | Путь | Тело запроса | Ответ |
|---|---|---|---|
| POST | `/api/v1/payments` | `{price}` | `201` `{paymentUid, status: "PAID", price}` |
| GET | `/api/v1/payments/{paymentUid}` | — | `200` `{paymentUid, status, price}` / `404` |
| DELETE | `/api/v1/payments/{paymentUid}` | — | `204` / `404` |

- `paymentUid` генерирует Payment.
- `DELETE` переводит статус в `CANCELED`.

### Loyalty Service (`:8050`)

| Метод | Путь | Заголовок | Ответ |
|---|---|---|---|
| GET | `/api/v1/loyalty` | `X-User-Name` | `200` `{status, discount, reservationCount}` |
| POST | `/api/v1/loyalty/increment` | `X-User-Name` | `200` `{status, discount, reservationCount}` |
| POST | `/api/v1/loyalty/decrement` | `X-User-Name` | `200` `{status, discount, reservationCount}` |

- Уровень по счётчику: `≥ 20` → `GOLD` 10%, `≥ 10` → `SILVER` 7%, иначе `BRONZE` 5%.
- Пользователя нет в таблице: `GET` отдаёт `BRONZE / 5 / 0` без вставки, `increment` делает upsert.
- `decrement` не опускает счётчик ниже 0.
- `increment`/`decrement` — действия, а не ресурс, осознанно: счётчик и статус меняются одним атомарным
  `UPDATE` внутри Loyalty, а не read-modify-write через Gateway.

## Сценарии Gateway

Обозначения: **R** — Reservation, **P** — Payment, **L** — Loyalty.

### `POST /api/v1/reservations` — забронировать

1. Валидация: `hotelUid` — UUID, даты в формате `YYYY-MM-DD`, `endDate > startDate`. Иначе `400` `ValidationErrorResponse`.
2. **R** `GET /hotels/{hotelUid}`. `404` → наружу `400` с `errors: [{field: "hotelUid", error: "hotel not found"}]`
   (в OpenAPI у этой ручки описан только `400`).
3. `nights = endDate − startDate`, `total = price × nights`.
4. **L** `GET /loyalty` → `discount`.
5. `paid = total × (100 − discount) / 100` (целочисленно; `payment.price` — `INT`).
6. **P** `POST /payments {price: paid}` → `paymentUid`.
7. **R** `POST /reservations {hotelUid, startDate, endDate, paymentUid}` → `reservationUid`.
   Бронь создаётся после платежа, потому что `payment_uid NOT NULL`.
8. **L** `POST /loyalty/increment`.
9. Ответ `200` `CreateReservationResponse`: `discount` — тот, что применён на шаге 4 (до инкремента).

Пример из тестов: 3 ночи × 10000, `GOLD` 10% → `27000`.

### `DELETE /api/v1/reservations/{uid}` — отменить

1. **R** `GET /reservations/{uid}` → `404` наружу как есть; иначе получаем `status` и `paymentUid`.
2. Если бронь уже `CANCELED` → `204` без дальнейших вызовов (повторная отмена не уменьшает счётчик дважды).
3. **R** `DELETE /reservations/{uid}`.
4. **P** `DELETE /payments/{paymentUid}`.
5. **L** `POST /loyalty/decrement`.
6. `204`.

### Чтение

| Внешняя ручка | Вызовы |
|---|---|
| `GET /hotels` | **R** `GET /hotels` — проксирование |
| `GET /reservations` | **R** `GET /reservations` → для каждой брони **P** `GET /payments/{paymentUid}` |
| `GET /reservations/{uid}` | **R** `GET /reservations/{uid}` (`404` наружу) → **P** `GET /payments/{paymentUid}` |
| `GET /me` | как `GET /reservations` + **L** `GET /loyalty` |
| `GET /loyalty` | **L** `GET /loyalty` — проксирование |

Gateway заменяет `paymentUid` на `payment: {status, price}`. Платежи запрашиваются по одному (N запросов) —
для объёмов лабы этого достаточно.

## Решения

- **Сбои посреди цепочки.** Компенсаций нет: ошибка или таймаут сервиса → `500` наружу.
  Шаги `book`/`cancel` упорядочены так, чтобы при сбое оставалось как можно меньше «висящих» данных.
- **Даты.** Колонки `start_date`/`end_data` в варианте — `TIMESTAMPTZ`; храним полночь UTC, отдаём `YYYY-MM-DD` в UTC.
  `end_data` — опечатка в варианте, таблицы создаём сами, поэтому называем колонку `end_date`.
- **Схема и тестовые данные.** Скрипт `postgres/scripts/db-v2.sql` создаёт только базы. Таблицы и сиды
  (отель и `Test Max`) каждый сервис создаёт при старте: `CREATE TABLE IF NOT EXISTS` +
  `INSERT … ON CONFLICT DO NOTHING`. DDL живёт рядом с кодом, который им пользуется, а таблицы принадлежат `program`.
  В Reservation `hotels` создаётся раньше `reservation` (FK). В `payment.payment_uid` добавляем `UNIQUE`.
- **Адреса сервисов в Gateway** — из env с fallback на `localhost`, как `DATABASE_URL` в конфигах сервисов.
  В `docker-compose.yml` у `gateway-service` нужны эти env и `depends_on` на три сервиса.

## Unit-тесты

- Gateway: `BookingService` с моками `I*Client` — расчёт цены и скидки, порядок вызовов в `book`,
  `cancel` уже отменённой брони, преобразование `404` от сервисов.
- Loyalty: `level_for` на границах (9/10, 19/20) и `decrement` ниже нуля.
- Репозитории и HTTP-клиенты unit-тестами не покрываем — их проверяют Postman-тесты в CI.
