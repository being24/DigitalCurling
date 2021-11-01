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
    class Buffer {
    public:
        Buffer(std::string_view prefix);
        Buffer(Buffer const&) = delete;
        Buffer & operator = (Buffer const&) = delete;
        ~Buffer();

        template <class T>
        Buffer & operator << (T&& value)
        {
            buffer_ << std::forward<T>(value);
            return *this;
        }

    private:
        std::ostringstream buffer_;
    };

    Log(std::ofstream && file);
    Log(Log const&) = delete;
    Log & operator = (Log const&) = delete;
    ~Log();

    /// <summary>
    /// 最も詳細なログ．主に通信内容の表示に用いる．
    /// </summary>
    /// <returns>ログ出力先バッファ</returns>
    static Buffer Trace();

    /// <summary>
    /// デバッグ用のログ．主にサーバーの状況を表示する．
    /// </summary>
    /// <returns>ログ出力先バッファ</returns>
    static Buffer Debug();

    /// <summary>
    /// GUIによるログ再生ではこのタイプのログを参照する．
    /// </summary>
    /// <returns>ログ出力先バッファ</returns>
    static Buffer Info();

    /// <summary>
    /// サーバーを終了する程では無い，異常な動作を検出した場合に使う．
    /// </summary>
    /// <returns>ログ出力先バッファ</returns>
    static Buffer Warn();

    /// <summary>
    /// 通信などにエラーが発生した場合用いる．このメッセージを出したらサーバーは速やかに終了する．
    /// </summary>
    /// <returns>ログ出力先バッファ</returns>
    static Buffer Error();

private:
    std::ofstream file_;
    std::mutex mutex_;
    static inline Log * instance_ = nullptr;
    static void PrintLine(std::string &&);
};


}

#endif
