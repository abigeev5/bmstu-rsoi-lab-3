#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "api/handlers.hpp"
#include "booking_component.hpp"

int main(int argc, char* argv[]) {
    const auto component_list = userver::components::MinimalServerComponentList()
                                    .Append<userver::server::handlers::Ping>()
                                    .Append<userver::components::TestsuiteSupport>()
                                    .AppendComponentList(userver::clients::http::ComponentList())
                                    .Append<gateway::BookingComponent>()
                                    .Append<gateway::HotelsHandler>()
                                    .Append<gateway::MeHandler>()
                                    .Append<gateway::ReservationsHandler>()
                                    .Append<gateway::ReservationHandler>()
                                    .Append<gateway::LoyaltyHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
