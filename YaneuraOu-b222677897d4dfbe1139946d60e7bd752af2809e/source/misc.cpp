﻿#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "misc.h"
#include "thread.h"

using namespace std;

Timer Time;

// --------------------
//  engine info
// --------------------

const string engine_info() {

  stringstream ss;
  
  ss << ENGINE_NAME << ' '
     << EVAL_TYPE_NAME << ' '
     << ENGINE_VERSION << setfill('0')
     << (Is64Bit ? " 64" : "32")
     << (use_avx2 ? " AVX2" : (use_sse42 ? " SSE4.2" : "")) << endl
     << "id author by yaneurao" << endl;

  return ss.str();
}

// --------------------
//  sync_out/sync_endl
// --------------------

std::ostream& operator<<(std::ostream& os, SyncCout sc) {
  static Mutex m;
  if (sc == IO_LOCK)    m.lock();
  if (sc == IO_UNLOCK)  m.unlock();
  return os;
}

// --------------------
//  logger
// --------------------

// logging用のhack。streambufをこれでhookしてしまえば追加コードなしで普通に
// cinからの入力とcoutへの出力をファイルにリダイレクトできる。
// cf. http://groups.google.com/group/comp.lang.c++/msg/1d941c0f26ea0d81
struct Tie : public streambuf
{
  Tie(streambuf* buf_ , streambuf* log_) : buf(buf_) , log(log_) {}

  int sync() { return log->pubsync(), buf->pubsync(); }
  int overflow(int c) { return write(buf->sputc((char)c), "<< "); }
  int underflow() { return buf->sgetc(); }
  int uflow() { return write(buf->sbumpc(), ">> "); }

  int write(int c, const char* prefix) {
    static int last = '\n';
    if (last == '\n')
      log->sputn(prefix, 3);
    return last = log->sputc((char)c);
  }

  streambuf *buf, *log; // 標準入出力 , ログファイル
};

struct Logger {
  static void start(bool b)
  {
    static Logger log;

    if (b && !log.file.is_open())
    {
      log.file.open("io_log.txt", ifstream::out);
      cin.rdbuf(&log.in);
      cout.rdbuf(&log.out);
      cout << "start logger" << endl;
    } else if (!b && log.file.is_open())
    {
      cout << "end logger" << endl;
      cout.rdbuf(log.out.buf);
      cin.rdbuf(log.in.buf);
      log.file.close();
    }
  }

private:
  Tie in, out;   // 標準入力とファイル、標準出力とファイルのひも付け
  ofstream file; // ログを書き出すファイル

  Logger() : in(cin.rdbuf(),file.rdbuf()) , out(cout.rdbuf(),file.rdbuf()) {}
  ~Logger() { start(false); }

};

void start_logger(bool b) { Logger::start(b); }

// --------------------
//  ファイルの丸読み
// --------------------

// ファイルを丸読みする。ファイルが存在しなくともエラーにはならない。空行はスキップする。
int read_all_lines(std::string filename, std::vector<std::string>& lines)
{
  fstream fs(filename,ios::in);
  if (fs.fail())
    return 1; // 読み込み失敗

  while (!fs.fail() && !fs.eof())
  {
    std::string line;
    getline(fs,line);
    if (line.length())
      lines.push_back(line);
  }
  fs.close();
  return 0;
}

