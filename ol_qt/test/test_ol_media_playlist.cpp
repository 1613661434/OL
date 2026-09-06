#include "ol_MediaPlaylist.h"

#include <QCoreApplication>
#include <QUrl>

#include <iostream>

namespace
{
    bool require(bool condition, const char* message)
    {
        if (condition) return true;
        std::cerr << message << '\n';
        return false;
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    ol::MediaPlaylist playlist;

    const QUrl first("file:///first.mp3");
    const QUrl second("file:///second.mp3");
    playlist.addMedia(first);
    playlist.addMedia(second);

    if (!require(playlist.getMediaCount() == 2, "playlist media count is incorrect")) return 1;
    if (!require(playlist.getCurrentIndex() == 0, "first media did not become current")) return 1;
    if (!require(playlist.getCurrentMedia() == first, "current media is incorrect")) return 1;
    if (!require(playlist.nextMedia() == second, "nextMedia did not select the second item")) return 1;
    if (!require(playlist.nextMedia() == first, "nextMedia did not wrap to the first item")) return 1;
    if (!require(playlist.previousMedia() == second, "previousMedia did not wrap to the last item")) return 1;

    playlist.clear();
    if (!require(playlist.getMediaCount() == 0, "clear did not remove all media")) return 1;
    if (!require(playlist.getCurrentIndex() == -1, "clear did not reset the current index")) return 1;
    if (!require(playlist.getCurrentMedia().isEmpty(), "empty playlist returned a current media URL")) return 1;

    std::cout << "MediaPlaylist regression test passed\n";
    return 0;
}
