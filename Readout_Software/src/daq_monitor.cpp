
// vmm
#include "daq_monitor.h"

// boost
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>

// std/stl
#include <iostream>

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  DaqMonitor
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
DaqMonitor::DaqMonitor(QObject* parent) :
    QObject(parent),
    m_dbg(true),
    n_interval_to_check(10),
    n_previous_counter(-1),
    n_live_counter( new int() ),
    m_io_service( new boost::asio::io_service ),
    m_worker( new boost::asio::io_service::work(*m_io_service) ),
    m_timer( new boost::asio::deadline_timer(*m_io_service) )
    
{
    (*n_live_counter) = 0;

}
DaqMonitor::~DaqMonitor()
{
    close_daq_monitor();
}
// ------------------------------------------------------------------------ //
void DaqMonitor::close_daq_monitor()
{
    std::cout << "DaqMonitor::close_daq_monitor()" << std::endl;
    m_io_service->stop();
    m_worker_threads.join_all();
    return;
}
// ------------------------------------------------------------------------ //
void DaqMonitor::OpenThread()
{
    m_worker_threads.create_thread(boost::bind(&DaqMonitor::MonitorThread, this));//, _1, m_io_service));
}
// ------------------------------------------------------------------------ //
//void DaqMonitor::MonitorThread( boost::shared_ptr< boost::asio::io_service > io_service )
void DaqMonitor::MonitorThread()
{
    while(true) {
        try {
            boost::system::error_code ec;
            m_io_service->run( ec );
            if( ec ) {
                std::cout << "DaqMonitor::MonitorThread [" << boost::this_thread::get_id()
                        << "] ERROR : " << ec << std::endl;
            }
            break;
        }//try
        catch( std::exception & ex ) {
            std::cout << "DaqMonitor::MonitorThread [" << boost::this_thread::get_id()
                    << "] Exception : " << ex.what() << std::endl;
        }// catch

    }// while
    std::cout << "DaqMonitor::MonitorThread [" << boost::this_thread::get_id()
            << "]  Thread finished" << std::endl;

}
// ------------------------------------------------------------------------ //
void DaqMonitor::CheckCount()// const boost::system::error_code & error)
{

 //   if(error) {
 //       std::cout << "DaqMonitor::CheckCount [" << boost::this_thread::get_id()
 //           << "] ERROR : " << error << std::endl;
 //   } // error
 //   else {
        std::cout << "DaqMonitor::CheckCount [" << boost::this_thread::get_id()
            << "]    counter_to_check: " << (*n_live_counter) << std::endl;


        // keep this loop going
        m_timer->expires_from_now(boost::posix_time::seconds(n_interval_to_check) );
        m_timer->async_wait(boost::bind(&DaqMonitor::CheckCount, this));
  //  }

}
// ------------------------------------------------------------------------ //
void DaqMonitor::begin()
{
    std::cout << "DaqMonitor::begin..." << std::endl;
    m_timer->expires_from_now(boost::posix_time::seconds(n_interval_to_check));
    m_timer->async_wait(boost::bind(&DaqMonitor::CheckCount, this));
    
}
// ------------------------------------------------------------------------ //
void DaqMonitor::stop()
{
    close_daq_monitor();
}
