#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>

namespace Ui {
class VideoPlayer;
}

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();
    void setVideo(QString filePath);
    void clearVideo();
    void stop();
    void play();
    void pause();
    void toggleRepeat();
private:
    Ui::VideoPlayer *ui;
    QMediaPlayer *player;
    QAudioOutput *audio;
    int sliderMove;
    QString videoLength;
    bool ignoreHours;
    bool repeat;

    void updateVolumeSlider(int value);
    void updateVideoProgress(qint64 position);
    void onDurationChange(qint64 duration);
    void updateVolumeIcon();
    // void onProgressMoved(int value);
    void onProgressReleased();
    void onProgressSliderAction(int action);
    void onProgressSliderMove(int position);
    void onTogglePlay();
    void onToggleMute();
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    QString formatTime(qint64 millis);
};

#endif // VIDEOPLAYER_H
