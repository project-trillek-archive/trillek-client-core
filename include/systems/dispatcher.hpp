#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <list>
#include <map>
#include <memory>
#include <mutex>

namespace trillek {
namespace event {

// Base class for data change subscriptions.
template <typename T>
class Subscriber {
public:
    virtual void Notify(const unsigned int entity_id, const T* data) { }
    virtual void Notify(const T* data) { }
};

// Dispatches data change notifications to the various subscribers.
template <typename T>
class Dispatcher {
private:
    Dispatcher() { }
    Dispatcher(const Dispatcher& right) {
        instance = right.instance;
    }
    Dispatcher& operator=(const Dispatcher& right) {
        if (this != &right) {
            instance = right.instance;
        }

        return *this;
    }
    static std::once_flag only_one;
    static std::shared_ptr<Dispatcher<T>> instance;
public:
    static std::shared_ptr<Dispatcher<T>> GetInstance() {
        std::call_once(Dispatcher::only_one,
            [ ] () {
            Dispatcher<T>::instance.reset(new Dispatcher<T>());
        }
        );

        return Dispatcher<T>::instance;
    }
    ~Dispatcher() { }

    /**
     * \brief Subscribes to be notified of data change events.
     *
     * \param const unsigned int entity_id ID of the entity to subscribe to.
     * \param const Subscriber<T>* subscriber The subscriber to add.
     * \return void
     */
    void Subscribe(const unsigned int entity_id, Subscriber<T>* subscriber) {
        auto sube = this->subcriberss.find(entity_id);
        if(sube == this->subcriberss.end()) {
            // no subscriptions for entity, add the entity and subscriber
            this->subcriberss[entity_id].push_back(subscriber);
            return;
        }
        else {
            // check if subscriber already exists
            for(auto sub : sube->second) {
                if(sub == subscriber) {
                    return; // already subscribed
                }
            }
            sube->second.push_back(subscriber);
        }
    }

    /**
     * \brief Subscribes to be notified of data change events for any entity ID.
     *
     * \param const Subscriber<T>* subscriber The subscriber to add.
     * \return void
     */
    void Subscribe(Subscriber<T>* subscriber) {
        this->subcriberss[0].push_back(subscriber);
    }

    /**
     * \brief Unsubscribes to notification of data change events.
     *
     * \param const unsigned int entity_id ID of the entity to unsubscribe from.
     * \param const Subscriber<T>* subscriber The subscriber to remove.
     * \return void
     */
    void Unsubscribe(const unsigned int entity_id, Subscriber<T>* subscriber)  {
        if (this->subcriberss.find(entity_id) != this->subcriberss.end()) {
            this->subcriberss[entity_id].remove(subscriber);
        }
    }

    /**
     * \brief Unsubscribes to notification of data change events.
     *
     * \param const Subscriber<T>* subscriber The subscriber to remove.
     * \return void
     */
    void Unsubscribe(Subscriber<T>* subscriber) {
        if (this->subcriberss.find(0) != this->subcriberss.end()) {
            this->subcriberss[0].remove(subscriber);
        }
    }

    /**
     * \brief Called to notify all subscribers that the data has changed.
     *
     * \param const unsigned int entity_id ID of the entity to update.
     * \param const T* data The changed data.
     * \return void
     */
    void NotifySubscribers(const unsigned int entity_id, const T* data) {
        if (this->subcriberss.find(entity_id) != this->subcriberss.end()) {
            auto subscriber_list = this->subcriberss.at(entity_id);
            for (Subscriber<T>* subscriber : subscriber_list) {
                subscriber->Notify(entity_id, data);
            }
        }
    }

    /**
     * \brief Called to notify all subscribers for entity ID 0 that the data has changed.
     *
     * \param const T* data The changed data.
     * \return void
     */
    void NotifySubscribers(const T* data) {
        if (this->subcriberss.find(0) != this->subcriberss.end()) {
            auto subscriber_list = this->subcriberss.at(0);
            for (Subscriber<T>* subscriber : subscriber_list) {
                subscriber->Notify(data);
            }
        }
    }
private:
    std::map<unsigned int, std::list<Subscriber<T>*>> subcriberss;
};

template<typename T>
std::once_flag Dispatcher<T>::only_one;

template<typename T>
std::shared_ptr<Dispatcher<T>> Dispatcher<T>::instance = nullptr;

} // End of event
} // End of trillek

#endif
