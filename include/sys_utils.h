#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace sys {

    namespace concurrency {

        using lock_t = std::unique_lock<std::mutex>;
        using task_t = std::function<void( )>;

        class notification_queue {
          public:
            void done( ) {
                {
                    lock_t lock{_mutex};
                    _done = true;
                }

                _ready.notify_all( );
            }

            bool try_pop( task_t &t ) {
                lock_t lock{_mutex, std::try_to_lock};

                if ( !lock || _q.empty( ) )
                    return false;

                t = move( _q.front( ) );
                _q.pop_front( );

                return true;
            }

            template <typename T> bool try_push( T &&t ) {
                {
                    lock_t lock{_mutex, std::try_to_lock};

                    if ( !lock )
                        return false;

                    _q.emplace_back( std::forward<T>( t ) );
                }

                _ready.notify_one( );
                return true;
            }

            bool pop( task_t &t ) {
                lock_t lock{_mutex};

                while ( _q.empty( ) && !_done )
                    _ready.wait( lock );

                if ( _q.empty( ) )
                    return false;

                t = move( _q.front( ) );
                _q.pop_front( );

                return true;
            }

            template <typename T> void push( T &&t ) {
                {
                    lock_t lock{_mutex};
                    _q.emplace_back( std::forward<T>( t ) );
                }
                _ready.notify_one( );
            }

          private:
            std::deque<task_t> _q;
            bool _done{false};
            std::mutex _mutex;
            std::condition_variable _ready;
        };

        class task_system {
          public:
            using count_t = unsigned int;

            explicit task_system( const count_t thread_count = std::thread::hardware_concurrency( ) ) : _count{thread_count} {
                _threads.reserve( thread_count );

                for ( count_t n = 0; n != _count; ++n ) {
                    _threads.emplace_back( [&, n] { run( n ); } );
                }
            }

            ~task_system( ) {
                for ( auto &e : _q )
                    e.done( );

                for ( auto &e : _threads )
                    if ( e.joinable( ) )
                        e.join( );
            }

            void run( const unsigned int i ) {
                while ( true ) {
                    task_t t;
                    if ( !_q[i].pop( t ) )
                        break;
                    t( );
                }
            }

            template <typename T> void async_( T &&t ) {
                auto i = _index++;

                for ( count_t n = 0; n != _count; ++n ) {
                    if ( _q[( i + n ) % _count].try_push( std::forward<T>( t ) ) )
                        return;
                }

                _q[i % _count].push( std::forward<T>( t ) );
            }

          private:
            const count_t _count;
            std::vector<std::thread> _threads;
            std::vector<notification_queue> _q{_count};
            std::atomic<count_t> _index{0};
        };

    } // namespace concurrency

    class thread_pool final {
      public:
        using task_t = std::function<void( )>;
        using lock_t = std::unique_lock<std::mutex>;
        using stop_flag_t = bool;
        using count_t = unsigned int;

        explicit thread_pool( const unsigned int thread_count = std::thread::hardware_concurrency( ) ) : _thread_count{thread_count} {
            workers.reserve( thread_count );

            for ( size_t i = 0; i < thread_count; i++ ) {
                workers.emplace_back( [this] {
                    while ( true ) {
                        task_t task;

                        {
                            lock_t lock{_queue_mutex};

                            if ( tasks.empty( ) )
                                _complete = true;

                            condition.wait( lock, [this] { return _stopped || !tasks.empty( ); } );

                            if ( _stopped && tasks.empty( ) )
                                return;

                            task = std::move( tasks.front( ) );
                            tasks.pop( );
                        }

                        task( );
                    }
                } );
            }
        }

        ~thread_pool( ) {

            {
                lock_t lock( _queue_mutex );
                _stopped = true;
            }

            condition.notify_all( );

            for ( auto &worker : workers )
                if ( worker.joinable( ) )
                    worker.join( );
        }

        template <class F, class... Args>
        auto enqueue( F &&f, Args &&... args ) -> std::future<typename std::result_of<F( Args... )>::type> {
            using return_type = typename std::result_of<F( Args... )>::type;
            auto task =
                std::make_shared<std::packaged_task<return_type( )>>( std::bind( std::forward<F>( f ), std::forward<Args>( args )... ) );
            std::future<return_type> res = task->get_future( );

            {
                lock_t lock{_queue_mutex};

                //                if ( stop )
                //                    throw std::runtime_error( "enqueue on stopped thread_pool" );

                tasks.emplace( [task] { ( *task )( ); } );
            }

            condition.notify_one( );
            return res;
        }

        void wait_until_complete( ) {
            using namespace std::chrono_literals;

            while ( true ) {
                std::this_thread::sleep_for( 100ms );

                if ( _complete )
                    break;
            }
        }

        thread_pool( const thread_pool & ) = delete;
        thread_pool &operator=( const thread_pool & ) = delete;

        thread_pool( thread_pool && ) = delete;
        thread_pool &operator=( const thread_pool && ) = delete;

        std::vector<std::thread> workers;
        std::queue<std::function<void( )>> tasks;

        std::mutex _queue_mutex;
        std::condition_variable condition;
        count_t _thread_count;
        stop_flag_t _stopped = false;
        std::atomic_bool _complete = false;
    };

} // namespace sys
