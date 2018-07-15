#ifndef TUXNET_LOCKABLE_H_INCLUDE
#define TUXNET_LOCKABLE_H_INCLUDE

#include <mutex>
#include <memory>

namespace tuxnet
{

    /**
     * @brief Wrapper for variables that sometimes need thread-safe access.
     *
     * Declare with:
     *
     * ```
     * lockable<some_type> my_variable(initial_value);
     * ```
     *
     * Use get() to access the variable without locking.
     *
     * atomic() can be used to perform atomic operations on the wrapped 
     * object, for instance using a lambda:
     *
     * ```
     * my_variable.atomic([some_type the_var]{ the_var.do_stuff() });
     * ```
     *
     * Simple locking and unlocking of the variable's underlying mutex 
     * can also still be done using the lock() and unlock() functions.
     *
     * Longer example, say you had a container you only want to lock
     * when writing:
     *
     * ```
     * typedef std::vector<std::string> strvector;
     * lockable<strvector> my_strings;
     * my_strings.get().push_back("foo");
     * my_strings.get().push_back("bar");
     * 
     * // Say that this is a function called from a thread that gets passed
     * // the above my_strings object.
     *
     * void do_work(const lockable<strvector>& the_strings)
     * {
     *     // Add another string in a way that is thread-safe.
     *     the_strings.atomic([](strvector& s){ s.push_back("baz") });
     * }
     *
     * ```
     */
    template <class T>
    class lockable
    {

        /// Underlying mutex.
        std::mutex m_lock;
        /// Stores a pointer to the managed object.
        std::unique_ptr<T> m_obj;

        public:
            
            /// Constructor.
            lockable(T obj)
            {
                m_obj = std::make_unique<T>(obj);
            };

            /// Get an instance of the managed object.
            T& get()
            {
                return *m_obj.get();
            };

            /**
             * @brief Perform an atomic operation on the object.
             *
             * For instance:
             * ```
             * my_variable.atomic([some_type the_var]{ the_var.do_stuff() });
             * ```
             *
             * The underlying mutex of the managed object will be locked before
             * calling the given function, and unlocked after executing the 
             * given function.
             *
             * @param f : Function to execute. It will be passed the managed
             *            object as first argument, and optionally any extra
             *            arguments defined.
             * @param args : Extra arguments to pass to the function.
             */
            template<typename Function, typename... Arguments>
            void atomic(Function f, Arguments... args)
            {
                lock();
                f(*m_obj.get(), args...);
                unlock();
            };

            /// Locks the underlying mutex of the managed object.
            void lock()
            {
                m_lock.lock();
            }

            /// Unlocks the underlying mutex of the managed object.
            void unlock()
            {
                m_lock.unlock();
            }
           
    };

}


#endif
