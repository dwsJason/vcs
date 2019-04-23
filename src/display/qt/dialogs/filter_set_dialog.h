#ifndef D_FILTER_SET_H
#define D_FILTER_SET_H

#include <QDialog>
#include "../../../filter/filter.h"
#include "../../../common/types.h"

class QTreeWidgetItem;
struct filter_set_s;

namespace Ui {
class FilterSetDialog;
}

class FilterSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterSetDialog(filter_set_s *const filterSet, QWidget *parent = 0, const bool allowApply = true);
    ~FilterSetDialog();

private slots:
    void on_radioButton_activeIn_toggled(bool checked);

    void on_checkBox_activeOut_toggled(bool checked);

    void on_pushButton_ok_clicked();

    void on_pushButton_cancel_clicked();

    void on_pushButton_apply_clicked();

private:
    filter_set_s make_filter_set_from_current_state(void);

    void apply_current_settings(void);

    Ui::FilterSetDialog *ui;

    // A pointer to the filter set that we'll modify.
    filter_set_s *const filterSet;

    // A copy of the original filter set, for canceling any edit we've made.
    filter_set_s backupFilterSet;
};

#endif // D_FILTER_SET_H
