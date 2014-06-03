#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

namespace trillek {

    class SystemBase {

    public:

        SystemBase() {};
        virtual ~SystemBase() {};

        /** \brief This function is executed when a thread is attached to the system
         *
         * \return virtual void
         *
         */
        virtual void ThreadInit() {};

        /** \brief Handle incoming events to update data
         *
         * This function is called once every frame. It is the only
         * function that can write data. This function is in the critical
         * path, job done here must be simple.
         *
         * If event handling need some batch processing, a task list must be
         * prepared and stored temporarily to be retrieved by RunBatch().
         *
         */
        virtual void HandleEvents(const frame_tp& timepoint) = 0;

        /** \brief Make all pre-update or post-update work.
         *
         * Pre-update work is prepared by HandleEvents() and executed here.
         * Updates must be done through events to be taken into account at
         * next frame.
         * Pre-update work is needed when data retrieved from event must be
         * processed (i.e make computation, reassemble, etc.) before being stored.
         *
         * Post-update work is the transformation of data to render entities or
         * other operations.
         *
         * Note that this function is const, i.e the system is in read-only mode.
         *
         */
        virtual void RunBatch() const = 0;

        /** \brief Save the data and terminate the system
         *
         * This function is called when the program is closing
         *
         */
        virtual void Terminate() = 0;
    };
}

#endif // SYSTEM_H_INCLUDED
