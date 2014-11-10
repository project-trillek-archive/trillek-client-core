#ifndef COMMANDQUEUE_HPP_INCLUDED
#define COMMANDQUEUE_HPP_INCLUDED

#include "atomic-map.hpp"

namespace trillek {

namespace component {
class Container;
}

/** \brief A storage for user commands
 */
class UserCommandQueue {
    typedef std::pair<id_t,std::shared_ptr<component::Container>> command_pair;
    typedef std::map<frame_tp,command_pair> usercommand_map_type;
    typedef usercommand_map_type::iterator command_iterator;
public:
    /** \brief Add a user command to the container
     *
     * The user command is added to a temporary list
     *
     * \param id entity id
     * \param user command std::shared_ptr<Container> the user command
     *
     */
    template<class T>
    void AddCommand(id_t id, T&& usercommand) const {
        temp_command_list.Insert(std::move(id), std::forward<T>(usercommand));
    };

    /** \brief Get the list of temporary user commands, and tag them
     *
     * \param from frame_tp inferior bound
     * \return a pair of iterators
     *
     */
    std::pair<command_iterator,command_iterator> GetAndTagCommandsFrom(frame_tp from) {
        for (auto& usercommand : temp_command_list.Poll()) {
            command_queue.insert(std::make_pair(std::move(from), std::move(usercommand)));
        }
        return command_queue.equal_range(from);
    };

    /** \brief Get a list of user commands between 2 timepoints
     *
     * User commands with a timepoint superior or equal to 'from' and
     * strictly inferior to 'to' are included
     *
     * \param from frame_tp inferior bound
     * \param to frame_tp superior bound
     * \return a pair of iterators
     *
     */
    std::pair<command_iterator,command_iterator> GetCommandsBetween(frame_tp from, frame_tp to);

    /** \brief Remove the user commands older than a timepoint
     *
     * User commands with a timepoint inferior or equal to timepoint are removed
     *
     * \param until frame_tp the superior bound
     *
     */
    void CleanCommandsUntil(frame_tp until);

private:
    // The underlying storage
    usercommand_map_type command_queue;
    // the temporary storage
    AtomicMap<id_t,std::shared_ptr<component::Container>> temp_command_list;
};

} //namespace trillek

#endif // COMMANDQUEUE_HPP_INCLUDED
