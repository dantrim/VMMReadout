
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

        void setCheckInterval(int interval = 10 /*seconds*/) { n_interval_to_check = interval; }

        // provide the counter from the outside world
        void setCounter(boost::shared_ptr< int > counter);// { n_live_counter = counter; }

        //void MonitorThread( boost::shared_ptr< boost::asio::io_service > io);
        void MonitorThread();
        void CheckCount();

        void OpenThread();

        void begin();

        bool isOn() { return m_is_monitoring; }


        // shut down the io_service and threads
        void close_daq_monitor();

    private :
        bool m_dbg;
        bool m_is_monitoring;
        int n_interval_to_check;
        int n_previous_counter;
        int n_stuck_count;
        boost::shared_ptr< int > n_live_counter;

        boost::shared_ptr< boost::asio::io_service > m_io_service;
        boost::shared_ptr< boost::asio::io_service::work > m_worker;
        boost::shared_ptr< boost::asio::deadline_timer > m_timer;

        boost::thread_group m_worker_threads;


    signals :
        void daqHangObserved();

    public slots :
        void stop();

}; // class

#endif
