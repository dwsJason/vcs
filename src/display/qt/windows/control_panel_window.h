/*
 * 2018 Tarpeeksi Hyvae Soft /
 * VCS
 *
 */

#ifndef D_CONTROL_PANEL_H
#define D_CONTROL_PANEL_H

#include <QDialog>
#include "common/types.h"

class ControlPanelRecordWidget;
class ControlPanelOutputWidget;
class ControlPanelAboutWidget;
class FilterSetsListDialog;
class VideoAndColorDialog;
class QTreeWidgetItem;
class AntiTearDialog;
class AliasDialog;
class MainWindow;

struct capture_signal_s;
struct resolution_s;
struct mode_alias_s;
struct log_entry_s;

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QDialog
{
    Q_OBJECT

public:
    explicit ControlPanel(MainWindow *const mainWin, QWidget *parent = 0);
    ~ControlPanel();

    void add_gui_log_entry(const log_entry_s &e);

    void update_capture_signal_info(void);

    void update_output_framerate(const uint fps, const bool hasMissedFrames);

    void update_output_resolution_info(void);

    void set_capture_info_as_no_signal();

    void set_capture_info_as_receiving_signal();

    void adjust_output_scaling(const int dir);

    bool is_mouse_wheel_scaling_allowed();

    QString GetString_OutputResolution();

    QString GetString_InputRefreshRate();

    QString GetString_InputResolution();

    QString GetString_OutputFrameRate();

    QString GetString_OutputLatency();

    QString GetString_DroppingFrames();

    void notify_of_new_alias(const mode_alias_s a);

    void clear_known_aliases();

    void notify_of_new_mode_settings_source_file(const QString &filename);

    void update_filter_set_idx(void);

    void open_video_adjust_dialog();

    void open_antitear_dialog();

    void open_filter_sets_dialog();

    void update_recording_metainfo();

    void toggle_overlay();

    void activate_capture_res_button(const uint buttonIdx);

    void update_video_params();

    void update_filter_sets_list();

    bool is_overlay_enabled(void);

    void update_stylesheet(const QString &stylesheet);

    bool custom_program_styling_enabled();

private slots:
    void on_checkBox_logInfo_toggled(bool);

    void on_checkBox_logDebug_toggled(bool);

    void on_checkBox_logErrors_toggled(bool);

    void parse_capture_resolution_button_press(QWidget *button);

    void on_comboBox_frameSkip_currentIndexChanged(const QString &arg1);

    void on_checkBox_logEnabled_stateChanged(int arg1);

    void on_pushButton_inputAdjustVideoColor_clicked();

    void on_comboBox_inputChannel_currentIndexChanged(int index);

    void on_pushButton_inputAliases_clicked();

    void on_comboBox_bitDepth_currentIndexChanged(const QString &arg1);

private:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *);

    void refresh_log_list_filtering();

    void connect_capture_resolution_buttons();

    void fill_capture_channel_combobox();

    void set_input_controls_enabled(const bool state);

    void filter_log_entry(QTreeWidgetItem *const entry);

    void reset_capture_bit_depth_combobox();

    void update_tab_widths();

    FilterSetsListDialog *filterSetsDlg = nullptr;
    VideoAndColorDialog *videocolorDlg = nullptr;
    AntiTearDialog *antitearDlg = nullptr;
    AliasDialog *aliasDlg = nullptr;

    ControlPanelAboutWidget *aboutWidget = nullptr;
    ControlPanelRecordWidget *recordWidget = nullptr;
    ControlPanelOutputWidget *outputWidget = nullptr;

    Ui::ControlPanel *ui;
};

#endif // D_CONTROL_PANEL_H
