#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QMainWindow>
#include <vector>

#include "buku.h"

class QButtonGroup;
class QComboBox;
class QGraphicsOpacityEffect;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QRadioButton;
class QSequentialAnimationGroup;
class QStackedWidget;
class QTableWidget;

class DashboardWindow : public QMainWindow
{
public:
    explicit DashboardWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *pages = nullptr;
    QLabel *topBreadcrumb = nullptr;
    QGraphicsOpacityEffect *pageOpacity = nullptr;
    QSequentialAnimationGroup *transitionAnimation = nullptr;

    std::vector<QPushButton *> navButtons;

    QTableWidget *bookTable = nullptr;
    QTableWidget *categoryTable = nullptr;
    QLineEdit *searchInput = nullptr;
    QComboBox *genreFilter = nullptr;
    QComboBox *sortChoice = nullptr;

    QLabel *totalBooksValue = nullptr;
    QLabel *totalGenresValue = nullptr;
    QLabel *totalAuthorsValue = nullptr;

    QComboBox *sortingField = nullptr;
    QComboBox *sortingOrder = nullptr;
    QButtonGroup *methodGroup = nullptr;
    QListWidget *beforePreview = nullptr;
    QListWidget *afterPreview = nullptr;
    QLabel *sortingMeta = nullptr;
    QLabel *sortingMethodBadge = nullptr;

    void createDashboardPage();
    void createBooksPage();
    void createCategoriesPage();
    void createSortingPage();

    void navigateToPage(int pageIndex, int navIndex = -1);
    void setActiveNavigation(int navIndex);

    void refreshDashboard();
    void refreshGenreFilter();
    void refreshBookTable();
    void refreshCategoryTable();
    void refreshSortingPreview(bool runAlgorithm);

    void sortBooks();
    void runSortingPreview();
    void resetSortingPreview();
    void showBookDialog(Buku *book = nullptr);
    void editSelectedBook();
    void deleteSelectedBook();
};

#endif
