/* 
 * File:   test_helper__restore_logger.h
 * Author: kjell
 *
 * Created on July 13, 2013, 4:46 PM
 */

#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <chrono>
#include <thread>
#include <algorithm>
#include "g2logworker.hpp"
#include "g2logmessage.hpp"
#include "g2filesink.hpp"

namespace testing_helpers {

   std::string mockFatalMessage();
   int mockFatalSignal();
   bool mockFatalWasCalled();
   void mockFatalCall(g2::FatalMessagePtr fatal_message);
   void clearMockFatal();

   bool removeFile(std::string path_to_file);
   bool verifyContent(const std::string &total_text, std::string msg_to_find);
   std::string readFileToText(std::string filename);
   
   
   
/** After initializing ScopedCout all std::couts is redirected to the buffer
 @verbatim
 Example: 
  stringstream buffer;   
  ScopedCout guard(&buffer);
  cout << "Hello World";
  ASSERT_STREQ(buffer.str().c_str(), "Hello World"); */
class ScopedOut {
  std::ostream& _out_type;
  std::streambuf* _old_cout;
public:
  explicit ScopedOut(std::ostream& out_type, std::stringstream* buffer)
     : _out_type(out_type)
     , _old_cout(_out_type.rdbuf()) {
      _out_type.rdbuf(buffer->rdbuf());
     }
      
  virtual ~ScopedOut() {
      _out_type.rdbuf(_old_cout);
   }
};

 
 
 


/// RAII cluttering files cleanup
class LogFileCleaner 
{
private:
  std::vector<std::string> logs_to_clean_;
  std::mutex g_mutex;
public:
  size_t size();
  LogFileCleaner() {}
  virtual ~LogFileCleaner();
  void addLogToClean(std::string path_to_log);
};


struct ScopedLogger {
    ScopedLogger();
    virtual ~ScopedLogger();
    
    g2::LogWorker* get();
    std::unique_ptr<g2::LogWorker> _currentWorker;
};




/** RAII temporarily replace of logger
 *  and restoration of original logger at scope end*/
struct RestoreFileLogger {
  explicit RestoreFileLogger(std::string directory);
  ~RestoreFileLogger();

  std::unique_ptr<ScopedLogger> _scope;
  void reset(){ _scope.reset();}
  

  template<typename Call, typename ... Args >
          typename std::result_of<Call(Args...)>::type callToLogger(Call call, Args&&... args) {
    auto func = std::bind(call, _scope->get(), std::forward<Args>(args)...);
    return func();
  }
  
  std::string logFile();
  std::string resetAndRetrieveContent();
  

private:
  std::unique_ptr<g2::SinkHandle<g2::FileSink>> _handle;
  std::string _log_file;
};





  typedef std::shared_ptr<std::atomic<bool>> AtomicBoolPtr;
  typedef std::shared_ptr<std::atomic<int>> AtomicIntPtr;
  struct ScopedSetTrue {
    AtomicBoolPtr  _flag;
    AtomicIntPtr _count;

    explicit ScopedSetTrue(AtomicBoolPtr flag, AtomicIntPtr count)
    : _flag(flag), _count(count) {
    }

    void ReceiveMsg(std::string message) {
      std::chrono::milliseconds wait{100};
      std::this_thread::sleep_for(wait);
      ++(*_count);
    }

    ~ScopedSetTrue() {
      (*_flag) = true;
    }
  };
} // testing_helpers




