#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "api/handlers.hpp"
#include "loyalty_component.hpp"

int main(int argc, char* argv[]) {
    const auto component_list = userver::components::MinimalServerComponentList()
                                    .Append<userver::server::handlers::Ping>()
                                    .Append<userver::components::TestsuiteSupport>()
                                    .Append<userver::components::Postgres>("postgres-db")
                                    .Append<loyalty::LoyaltyComponent>()
                                    .Append<loyalty::LoyaltyHandler>()
                                    .Append<loyalty::LoyaltyCounterHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
