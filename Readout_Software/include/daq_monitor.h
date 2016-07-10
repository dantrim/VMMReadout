
#ifndef DAQ_MONITOR_H
#define DAQ_MONITOR_H

// std/stl

// vmm

// qt
#include <QObject>

// boost
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>


class DaqMonitor : public QObject
{
    Q_OBJECT

    public :
        explicit DaqMonitor(QObject *parent = 0);
        virtual ~DaqMonitor();

        void setCounter(boost::shared_ptr< int > counter) { n_live_counter = counter; }

        //void MonitorThread( boost::shared_ptr< boost::asio::io_service > io);
        void MonitorThread();
        //void CheckCount(const boost::system::error_code & err);
        void CheckCount();

        //static void CheckCount(const boost::system::error_code & error, //);//,
        //            boost::shared_ptr< boost::asio::deadline_timer > timer,
        //            boost::shared_ptr< int > counter );

        void OpenThread();

        void begin();


        // shut down the io_service and threads
        void close_daq_monitor();

    private :
        bool m_dbg;
        int n_previous_counter;
        boost::shared_ptr< int > n_live_counter;

        boost::shared_ptr< boost::asio::io_service > m_io_service;
        boost::shared_ptr< boost::asio::io_service::work > m_worker;
        boost::shared_ptr< boost::asio::deadline_timer > m_timer;

        boost::thread_group m_worker_threads;


    signals :

    public slots :
        void stop();

}; // class

#endif
