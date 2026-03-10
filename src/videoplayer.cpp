#include "videoplayer.h"
#include "ui_videoplayer.h"
#include <QSettings>

VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoPlayer), ignoreHours(false), repeat(false)
{
    ui->setupUi(this);
    player = new QMediaPlayer;
    audio = new QAudioOutput;
    sliderMove = -1;
    player->setAudioOutput(audio);
    player->setVideoOutput(ui->videoWidget);
    connect(ui->playButton, &QToolButton::clicked, this, &VideoPlayer::onTogglePlay);
    connect(ui->muteButton, &QPushButton::clicked, this, &VideoPlayer::onToggleMute);
    connect(ui->volumeSlider, &QSlider::valueChanged, this, &VideoPlayer::updateVolumeSlider);
    connect(player, &QMediaPlayer::durationChanged, this, &VideoPlayer::onDurationChange);
    connect(player, &QMediaPlayer::positionChanged, this, &VideoPlayer::updateVideoProgress);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &VideoPlayer::onPlaybackStateChanged);
    connect(ui->progressSlider, &QSlider::sliderReleased, this, &VideoPlayer::onProgressReleased);
    connect(ui->progressSlider, &QSlider::sliderMoved, this, &VideoPlayer::onProgressSliderMove);
    connect(ui->progressSlider, &QSlider::actionTriggered, this, &VideoPlayer::onProgressSliderAction);
    connect(ui->repeatButton, &QPushButton::clicked, this, &VideoPlayer::toggleRepeat);
    QSettings settings("MyApp","Tag Viewer");
    settings.beginGroup("video");
    int volume = settings.value("volume", 75).toInt();
    audio->setVolume(float(volume) / 100.0);
    ui->volumeSlider->setValue(volume);
    repeat = settings.value("repeat", false).toBool();
    player->setLoops(repeat ? QMediaPlayer::Infinite : 1);
    ui->repeatButton->setIcon(QIcon(repeat ? ":/icons/linear/repeat" : ":/icons/linear/repeat-disabled"));
    bool muted = settings.value("muted", false).toBool();
    audio->setMuted(muted);
    settings.endGroup();
    ui->volumeSlider->setEnabled(!muted);
    updateVolumeIcon();
}

void VideoPlayer::stop() {
    player->stop();
}

void VideoPlayer::play() {
    if (player->playbackState() == QMediaPlayer::StoppedState)
        ui->progressSlider->setValue(0);
    player->play();
}

void VideoPlayer::toggleRepeat() {
    repeat = !repeat;
    player->setLoops(repeat ? QMediaPlayer::Infinite : 1);
    ui->repeatButton->setIcon(QIcon(repeat ? ":/icons/linear/repeat" : ":/icons/linear/repeat-disabled"));
    ui->repeatButton->clearFocus();
}

void VideoPlayer::pause() {
    player->pause();
}
void VideoPlayer::updateVolumeSlider(int value) {
    audio->setVolume(float(value) / 100.0);
    updateVolumeIcon();
}

void VideoPlayer::updateVolumeIcon() {
    QIcon::ThemeIcon icon;
    if (audio->isMuted())
        icon = QIcon::ThemeIcon::AudioVolumeMuted;
    else {
        int volume = ui->volumeSlider->value();
        if (volume < 67) {
            if (volume < 34)
                icon = QIcon::ThemeIcon::AudioVolumeLow;
            else
                icon = QIcon::ThemeIcon::AudioVolumeMedium;
        } else
            icon = QIcon::ThemeIcon::AudioVolumeHigh;
    }

    ui->muteButton->setIcon(QIcon::fromTheme(icon));
}

void VideoPlayer::updateVideoProgress(qint64 position) {
    QString progress = formatTime(position);
    ui->timerLabel->setText(progress + " / " + videoLength);
    if (!ui->progressSlider->isSliderDown()) {
        ui->progressSlider->setValue(int(position / 500));
    }
}

void VideoPlayer::onDurationChange(qint64 duration) {
    ignoreHours = (duration/1000/60/60 == 0);
    videoLength = formatTime(duration);

    ui->progressSlider->setMaximum(int(duration/500));
}

void VideoPlayer::onProgressReleased() {
    if (sliderMove != -1) {
        ui->progressSlider->setValue(sliderMove);
        player->setPosition(sliderMove*500);
        sliderMove = -1;
    }
    else {
        ui->progressSlider->setValue(player->position() / 500);
    }
}

void VideoPlayer::onProgressSliderMove(int position) {
    sliderMove = position;
}

void VideoPlayer::onProgressSliderAction(int action) {
    switch(action) {
    case QSlider::SliderPageStepAdd:
    case QSlider::SliderPageStepSub:
    case QSlider::SliderSingleStepAdd:
    case QSlider::SliderSingleStepSub:
    case QSlider::SliderToMaximum:
    case QSlider::SliderToMinimum:
        ui->progressSlider->setSliderPosition(ui->progressSlider->value());
        break;
    }
}

void VideoPlayer::onTogglePlay() {
    QMediaPlayer::PlaybackState state = player->playbackState();
    if (state == QMediaPlayer::PlayingState)
        pause();
    else if (state == QMediaPlayer::PausedState || state == QMediaPlayer::StoppedState)
        play();
}

void VideoPlayer::onToggleMute() {
    bool muted = audio->isMuted();

    audio->setMuted(!muted);
    ui->volumeSlider->setEnabled(muted);
    updateVolumeIcon();
}

void VideoPlayer::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
    if (state == QMediaPlayer::PlayingState)
        ui->playButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
    else if (state == QMediaPlayer::PausedState || state == QMediaPlayer::StoppedState)
        ui->playButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
}
void VideoPlayer::setVideo(QString filePath) {
    player->setSource(QUrl(filePath));
    player->setPosition(0);

}

QString VideoPlayer::formatTime(qint64 millis) {
    int seconds = millis / 1000;
    int minutes = seconds / 60;
    int hours = minutes / 60;
    QString ret = QString::number(minutes%60).rightJustified(2, '0') + ':' + QString::number(seconds%60).rightJustified(2, '0');
    if (!ignoreHours)
        ret = QString::number(hours) + ':' + ret;
    return ret;
}

void VideoPlayer::clearVideo() {
    player->stop();
    player->setSource(QUrl());
}

VideoPlayer::~VideoPlayer()
{
    player->stop();
    QSettings settings("MyApp","Tag Viewer");
    settings.beginGroup("video");
    settings.setValue("volume", ui->volumeSlider->value());
    settings.setValue("muted", audio->isMuted());
    settings.setValue("repeat", repeat);
    settings.endGroup();
    delete ui;
}
