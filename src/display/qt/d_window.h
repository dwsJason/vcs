/*
 * 2018 Tarpeeksi Hyvae Soft /
 * VCS
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../display.h"
#include "../../common/globals.h"

class OverlayDialog;
class ControlPanel;
class PerfDialog;

struct capture_signal_s;
struct mode_alias_s;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // Gets user input in the GUI, etc.
    void update_gui_state();

    // Returns true if the window has a border.
    bool window_has_border();

    u32 ShowMessageBox_Query(const QString title, const QString msg);

    void ShowMessageBox_Information(const QString msg);

    void ShowMessageBox_Error(const QString msg);

    void add_gui_log_entry(const log_entry_s e);

    void update_capture_signal_info(void);

    void update_output_framerate(const u32 fps, const bool missedFrames);

    void set_capture_info_as_no_signal();

    void set_capture_info_as_receiving_signal();

    void update_window_size();

    void update_window_title();

    QString GetString_OutputResolution();

    QString GetString_InputResolution();

    QString GetString_InputRefreshRate();

    QString GetString_OutputFrameRate();

    QString GetString_OutputLatency();

    QString GetString_DroppingFrames();

    void signal_new_known_mode(const resolution_s r);

    void clear_known_modes();

    void signal_new_known_alias(const mode_alias_s a);

    void clear_known_aliases();

    void show_overlay_dialog();

    void set_overlay_enabled(const bool state);

    void signal_that_overlay_is_enabled(const bool enabled);

    void measure_framerate();

    void signal_new_mode_settings_source_file(const std::__cxx11::string &filename);

    void update_current_filter_set_idx(const int idx);

    void set_opengl_enabled(const bool enabled);

    void refresh();

    QImage overlay_image();

    void update_recording_metainfo();

    void update_video_params();

private slots:
    void toggle_window_border();

private:
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *);
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void changeEvent(QEvent *event);

    void set_keyboard_shortcuts();

    Ui::MainWindow *ui = nullptr;

    ControlPanel *controlPanel = nullptr;
    OverlayDialog *overlayDlg = nullptr;

    // Set to true when the user has selected to close the
    // main window.
    bool userExiting = false;

    // Set to true if the program failed to initialize the capture card.
    bool captureInitFail = false;
};

#endif // MAINWINDOW_H
