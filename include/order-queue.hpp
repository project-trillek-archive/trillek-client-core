#ifndef ORDERQUEUE_HPP_INCLUDED
#define ORDERQUEUE_HPP_INCLUDED

#include "atomic-map.hpp"

namespace trillek {

namespace component {
class Container;
}

/** \brief A storage for orders
 */
class OrderQueue {
    typedef std::pair<id_t,std::shared_ptr<component::Container>> order_pair;
    typedef std::map<frame_tp,order_pair> order_map_type;
    typedef order_map_type::iterator order_iterator;
public:
    /** \brief Add an order to the container
     *
     * The order is added to a temporary list
     *
     * \param id entity id
     * \param order std::shared_ptr<Container> the order
     *
     */
    template<class T>
    void AddOrder(id_t id, T&& order) const {
        temp_order_list.Insert(std::move(id), std::forward<T>(order));
    };

    /** \brief Get the list of temporary orders, and tag them
     *
     * \param from frame_tp inferior bound
     * \return the orders
     *
     */
    std::pair<order_iterator,order_iterator> GetAndTagOrdersFrom(frame_tp from) {
        for (auto& order : temp_order_list.Poll()) {
            order_queue.insert(std::make_pair(std::move(from), std::move(order)));
        }
        return order_queue.equal_range(from);
    };

    /** \brief Get a list of orders between 2 timepoints
     *
     * Orders with a timepoint superior or equal to 'from' and
     * strictly inferior to 'to' are included
     *
     * \param from frame_tp inferior bound
     * \param to frame_tp superior bound
     * \return std::unique_ptr<order_map_type> the orders
     *
     */
    std::unique_ptr<order_map_type> GetOrdersBetween(frame_tp from, frame_tp to);

    /** \brief Remove the orders older than a timepoint
     *
     * Orders with a timepoint inferior or equal to timepoint are removed
     *
     * \param until frame_tp the superior bound
     * \return std::unique_ptr<order_map_type> the orders
     *
     */
    std::unique_ptr<order_map_type> CleanOrdersUntil(frame_tp until);

private:
    /** \brief Tag the orders with a timestamp
     *
     * All the orders in temporary lists are tagged and
     * moved to the main container
     *
     * \param timepoint frame_tp the timestamp
     *
     */
    void TagOrders(frame_tp timepoint);

    // The underlying storage
    order_map_type order_queue;
    // the temporary storage
    AtomicMap<id_t,std::shared_ptr<component::Container>> temp_order_list;
};

} //namespace trillek

#endif // ORDERQUEUE_HPP_INCLUDED
