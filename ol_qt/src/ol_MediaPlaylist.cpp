/****************************************************************************************/
/*
 * 程序名：ol_MediaPlaylist.cpp
 * 功能描述：媒体播放列表类实现文件
 * 作者：ol
 */
/****************************************************************************************/
#include "ol_MediaPlaylist.h"
#include <QMediaPlayer>

namespace ol
{
    MediaPlaylist::MediaPlaylist(QObject* parent)
        : QObject(parent), m_currentIndex(-1), m_playbackMode(PlaybackMode::CurrentItemOnce), m_player(nullptr)
    {
    }

    void MediaPlaylist::addMedia(const QUrl& media)
    {
        m_mediaList.append(media);
        if (m_currentIndex == -1) setCurrentIndex(0);
    }

    void MediaPlaylist::clear()
    {
        m_mediaList.clear();
        m_currentIndex = -1;
        emit currentIndexChanged(m_currentIndex);
    }

    void MediaPlaylist::setCurrentIndex(int index)
    {
        if (index >= 0 && index < m_mediaList.size() && index != m_currentIndex)
        {
            m_currentIndex = index;
            emit currentIndexChanged(m_currentIndex);

            // 自动切换播放器音源
            if (m_player) m_player->setSource(getCurrentMedia());
        }
    }

    QUrl MediaPlaylist::getCurrentMedia() const
    {
        if (m_currentIndex >= 0 && m_currentIndex < m_mediaList.size()) return m_mediaList.at(m_currentIndex);
        return QUrl();
    }

    void MediaPlaylist::setPlayer(QMediaPlayer* player)
    {
        // 断开旧播放器连接
        if (m_player) disconnect(m_player, nullptr, this, nullptr);

        m_player = player;

        // 绑定状态监听信号
        if (m_player)
        {
            connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MediaPlaylist::onMediaStatusChanged);

            // 默认加载当前音源
            const QUrl url = getCurrentMedia();
            if (url.isValid()) m_player->setSource(url);
        }
    }

    QUrl MediaPlaylist::nextMedia()
    {
        if (m_mediaList.isEmpty()) return QUrl();

        setCurrentIndex((m_currentIndex + 1) % m_mediaList.size());
        return getCurrentMedia();
    }

    QUrl MediaPlaylist::previousMedia()
    {
        if (m_mediaList.isEmpty()) return QUrl();

        setCurrentIndex((m_currentIndex - 1 + m_mediaList.size()) % m_mediaList.size());
        return getCurrentMedia();
    }

    void MediaPlaylist::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
    {
        // 仅处理播放完成状态
        if (status != QMediaPlayer::EndOfMedia || !m_player) return;
        if (m_mediaList.isEmpty()) return;

        switch (m_playbackMode)
        {
        case PlaybackMode::CurrentItemOnce:
            m_player->stop();
            break;
        case PlaybackMode::CurrentItemLoop:
            m_player->play();
            break;
        case PlaybackMode::SequentialLoop:
            nextMedia();
            m_player->play();
            break;
        }
    }

} // namespace ol