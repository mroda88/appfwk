/**
 * @file DAQProcess class interface
 *
 * DAQProcess is the central container for instantiated UserModules and Buffers within a DAQ Application.
 * It loads a ModuleList which defines the graph of UserModules and Buffers and any command ordering.
 * DAQProcess is responsible for distributing commands recieved from CCM to the UserModules in the order
 * defined.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef app_framework_DAQProcess_hh
#define app_framework_DAQProcess_hh

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/Core/ModuleList.hh"
#include "app-framework-base/UserModules/UserModule.hh"

#include <map>
#include <string>

namespace appframework {
/**
 * @brief The DAQProcess class is the central container for UserModules and Buffers.
 *
 * DAQProcess receives commands from CCM and distributes them to the UserModules in the order defined 
 * in the CommandOrderMap received from the ModuleList during register_modules.
 */
class DAQProcess {
   public:
    /**
     * @brief DAQProcess Constructor
     * @param args Command-line arguments to the DAQ Application
     * 
     * The DAQProcess constructor instantiates essential DAQ Application services. Services are passed
     * the command-line options and may also read basic configuration from the environment.
     */
    DAQProcess(std::list<std::string> args);
    /**
     * @brief Using the given ModuleList, construct the graph of UserModules and Buffers
     * @param ml ModuleList to call ModuleList::ConstructGraph on
     *
     * The register_modules function calls the ModuleList's ConstructGraph function, which instantiates and
     * links together the UserModules and Buffers needed by this DAQ Application. ConstructGraph also defines
     * any ordering of commands for UserModules.
     */
    void register_modules(std::unique_ptr<ModuleList> const& ml);
    /**
     * @brief Execute the specified command on the loaded UserModules
     * @param cmd Command to execute
     *
     * This function will determine if there is an entry in the command order map for this command, and
     * if so, first send the command to the UserModules in that list in the order specified. Then, any
     * remaining UserModules will receive the command in an unspecified order.
     */
    void execute_command(const std::string& );
    /**
     * @brief Start the CommandFacility listener
     * @return Return code from listener
     *
     * This function should call the loaded CommandFacility::listen method, which should block for the
     * duration of the DAQ Application, calling execute_command as necessary.
     */
    int listen();

   private:

    BufferMap bufferMap_;                    ///< String alias for each Buffer
    UserModuleMap userModuleMap_;            ///< String alias for each UserModule
    CommandOrderMap commandOrderMap_;        ///< Order UserModule commands by alias
};
}  // namespace appframework

#endif  // app_framework_DAQProcess_hh