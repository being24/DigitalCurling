#ifndef DIGITAL_CURLING_SERVER_LOG_HPP
#define DIGITAL_CURLING_SERVER_LOG_HPP

#include <string_view>
#include <fstream>
#include <sstream>
#include <mutex>

namespace digital_curling::server {

/// <summary>
/// サーバーのログはこのクラスの関数を介して出力される．
/// シングルトン．
/// </summary>
class Log {
public:

    Log(std::ofstream && file);
    Log(Log const&) = delete;
    Log & operator = (Log const&) = delete;
    ~Log();

    /// <summary>
    /// 最も詳細なログ．主に通信内容の表示に用いる．
    /// </summary>
    /// <param name="message">出力されるメッセージ</param>
    static void Trace(std::string_view message);

    static void Trace(std::ostringstream const& message)
    {
        Trace(message.str());
    }

    /// <summary>
    /// デバッグ用のログ．主にサーバーの状況を表示する．
    /// </summary>
    /// <param name="message">出力されるメッセージ</param>
    static void Debug(std::string_view message);

    static void Debug(std::ostringstream const& message)
    {
        Debug(message.str());
    }

    /// <summary>
    /// GUIによるログ再生ではこのタイプのログを参照する．
    /// </summary>
    /// <param name="message">出力されるメッセージ</param>
    static void Info(std::string_view message);

    static void Info(std::ostringstream const& message)
    {
        Info(message.str());
    }

    /// <summary>
    /// サーバーを終了する程では無い，異常な動作を検出した場合に使う．
    /// </summary>
    /// <param name="message">出力されるメッセージ</param>
    static void Warn(std::string_view message);

    static void Warn(std::ostringstream const& message)
    {
        Warn(message.str());
    }

    /// <summary>
    /// 通信などにエラーが発生した場合用いる．このメッセージを出したらサーバーは速やかに終了する．
    /// </summary>
    /// <param name="message">出力されるメッセージ</param>
    static void Error(std::string_view message);

    static void Error(std::ostringstream const& message)
    {
        Error(message.str());
    }

private:
    std::ofstream file_;
    std::mutex mutex_;
    static inline Log * instance_ = nullptr;
    static void Print(char const * prefix, std::string_view message);
};


}

#endif
