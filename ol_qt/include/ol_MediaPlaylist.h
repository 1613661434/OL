/****************************************************************************************/
/*
 * 程序名：ol_MediaPlaylist.h
 * 功能描述：Qt6 媒体播放列表类，替代废弃的 QMediaPlaylist，支持以下特性：
 *          - 自动绑定 QMediaPlayer，监听播放完成状态
 *          - 三种播放模式：单次播放、单曲循环、列表循环
 *          - 播放结束自动切换曲目/循环播放
 *          - 索引/媒体资源自动管理，适配 Qt6
 *          - 支持信号通知索引变化，与音频播放器联动
 * 作者：ol
 * 适用标准：Qt 6.0及以上
 */
/****************************************************************************************/
#ifndef OL_MEDIAPLAYLIST_H
#define OL_MEDIAPLAYLIST_H 1

#include <QObject>
#include <QVector>
#include <QUrl>
#include <QMediaPlayer>

namespace ol
{

    /**
     * @brief 媒体播放列表类，适配 Qt6 QMediaPlayer
     * 实现播放列表管理、自动循环切歌、播放模式控制，无需手动处理音源切换
     */
    class MediaPlaylist : public QObject
    {
        Q_OBJECT
    public:
        /**
         * @brief 播放模式枚举
         */
        enum class PlaybackMode : char
        {
            CurrentItemOnce, /**< 当前曲目播放一次后自动停止 */
            CurrentItemLoop, /**< 当前曲目无限循环播放 */
            SequentialLoop   /**< 列表顺序循环，尾首自动衔接 */
        };

    private:
        QVector<QUrl> m_mediaList;   ///< 媒体资源 URL 列表
        int m_currentIndex;          ///< 当前播放索引，-1 表示未选中
        PlaybackMode m_playbackMode; ///< 当前播放模式
        QMediaPlayer* m_player;      ///< 绑定的 Qt 媒体播放器

    public:
        /**
         * @brief 构造函数
         * @param parent 父对象
         */
        explicit MediaPlaylist(QObject* parent = nullptr);

        /**
         * @brief 向播放列表添加媒体资源
         * @param media 媒体资源 URL（支持 qrc 资源/本地文件）
         */
        void addMedia(const QUrl& media);

        /**
         * @brief 清空播放列表，重置索引
         */
        void clear();

        /**
         * @brief 设置当前播放索引（自动切换播放器音源）
         * @param index 目标播放索引
         */
        void setCurrentIndex(int index);

        /**
         * @brief 获取当前播放索引
         * @return 当前索引，-1 表示无效
         */
        inline int getCurrentIndex() const { return m_currentIndex; }

        /**
         * @brief 获取当前播放的媒体 URL
         * @return 有效 URL / 空 QUrl
         */
        QUrl getCurrentMedia() const;

        /**
         * @brief 获取列表总媒体数量
         * @return 媒体资源总数
         */
        inline qsizetype getMediaCount() const { return m_mediaList.size(); }

        /**
         * @brief 设置播放模式
         * @param mode 目标播放模式
         */
        inline void setPlaybackMode(PlaybackMode mode) { m_playbackMode = mode; }

        /**
         * @brief 获取当前播放模式
         * @return 播放模式枚举值
         */
        inline PlaybackMode getPlaybackMode() const { return m_playbackMode; }

        /**
         * @brief 绑定 Qt 媒体播放器
         * @param player 待绑定的 QMediaPlayer 对象
         */
        void setPlayer(QMediaPlayer* player);

        /**
         * @brief 切换到下一首媒体
         * @return 下一首媒体的 URL
         */
        QUrl nextMedia();

        /**
         * @brief 切换到上一首媒体
         * @return 上一首媒体的 URL
         */
        QUrl previousMedia();

    signals:
        /**
         * @brief 播放索引改变信号，用于用户未来功能扩展
         * @param newIndex 新的播放索引
         */
        void currentIndexChanged(int newIndex);

    private slots:
        /**
         * @brief 监听播放器状态，播放完成自动处理切歌/循环
         * @param status 媒体播放状态
         */
        void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    };

} // namespace ol

#endif // !OL_MEDIAPLAYLIST_H