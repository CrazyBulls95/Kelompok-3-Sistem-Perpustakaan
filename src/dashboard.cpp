#include "dashboard.h"

#include <QAbstractItemView>
#include <QButtonGroup>
#include <QByteArray>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QEasingCurve>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSequentialAnimationGroup>
#include <QSignalBlocker>
#include <QStyle>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace {

QString toQString(const std::string &value)
{
    return QString::fromUtf8(value.c_str(), static_cast<int>(value.size()));
}

std::string toStdString(const QString &value)
{
    const QByteArray utf8 = value.toUtf8();
    return std::string(utf8.constData(), static_cast<size_t>(utf8.size()));
}

QPushButton *makeButton(const QString &text, const QString &objectName)
{
    auto *button = new QPushButton(text);
    button->setObjectName(objectName);
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(38);
    button->setFocusPolicy(Qt::StrongFocus);
    return button;
}

QLabel *makeFieldLabel(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName("fieldLabel");
    return label;
}

QFrame *makeSurface(const QString &objectName = "surfaceCard")
{
    auto *frame = new QFrame;
    frame->setObjectName(objectName);
    frame->setFrameShape(QFrame::NoFrame);
    return frame;
}

QWidget *makeStatCard(const QString &symbol, const QString &caption, QLabel **valueLabel,
                      const QString &hint)
{
    auto *card = makeSurface("statCard");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 15);
    layout->setSpacing(7);

    auto *top = new QHBoxLayout;
    auto *icon = new QLabel(symbol);
    icon->setObjectName("statIcon");
    icon->setAlignment(Qt::AlignCenter);
    icon->setFixedSize(34, 34);
    top->addWidget(icon);
    top->addStretch();
    auto *dot = new QLabel("●");
    dot->setObjectName("statDot");
    top->addWidget(dot);

    auto *captionLabel = new QLabel(caption.toUpper());
    captionLabel->setObjectName("statCaption");

    *valueLabel = new QLabel("0");
    (*valueLabel)->setObjectName("statValue");

    auto *hintLabel = new QLabel(hint);
    hintLabel->setObjectName("statHint");

    layout->addLayout(top);
    layout->addWidget(captionLabel);
    layout->addWidget(*valueLabel);
    layout->addWidget(hintLabel);
    return card;
}

QFrame *makeMethodCard(const QString &iconText, const QString &title,
                       const QString &description, const QString &fieldHint,
                       QButtonGroup *group, int methodId, QRadioButton **radioOut)
{
    auto *card = makeSurface("methodCard");
    card->setProperty("active", false);
    card->setMinimumHeight(122);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(5);

    auto *top = new QHBoxLayout;
    auto *icon = new QLabel(iconText);
    icon->setObjectName("methodIcon");
    icon->setAlignment(Qt::AlignCenter);
    icon->setFixedSize(30, 30);
    top->addWidget(icon);
    top->addStretch();

    auto *radio = new QRadioButton;
    radio->setObjectName("methodRadio");
    radio->setCursor(Qt::PointingHandCursor);
    group->addButton(radio, methodId);
    top->addWidget(radio, 0, Qt::AlignTop);
    layout->addLayout(top);

    auto *heading = new QLabel(title);
    heading->setObjectName("methodTitle");
    auto *body = new QLabel(description);
    body->setObjectName("methodDescription");
    body->setWordWrap(true);
    auto *hint = new QLabel(fieldHint);
    hint->setObjectName("methodHint");

    layout->addWidget(heading);
    layout->addWidget(body, 1);
    layout->addWidget(hint);

    QObject::connect(radio, &QRadioButton::toggled, card, [card](bool checked) {
        card->setProperty("active", checked);
        card->style()->unpolish(card);
        card->style()->polish(card);
        card->update();
    });

    *radioOut = radio;
    return card;
}

QString bookDateText(const Buku &book)
{
    return QString("%1/%2/%3")
        .arg(book.tanggal, 2, 10, QLatin1Char('0'))
        .arg(book.bulan, 2, 10, QLatin1Char('0'))
        .arg(book.tahun, 4, 10, QLatin1Char('0'));
}

bool bookLess(const Buku &left, const Buku &right, const QString &field, bool ascending)
{
    int comparison = 0;
    if (field == "judul") {
        comparison = QString::compare(toQString(left.judul), toQString(right.judul), Qt::CaseInsensitive);
    } else if (field == "penulis") {
        comparison = QString::compare(toQString(left.penulis), toQString(right.penulis), Qt::CaseInsensitive);
    } else if (field == "jenis") {
        comparison = QString::compare(toQString(left.jenis), toQString(right.jenis), Qt::CaseInsensitive);
    } else if (field == "tanggal") {
        const auto leftDate = std::make_tuple(left.tahun, left.bulan, left.tanggal);
        const auto rightDate = std::make_tuple(right.tahun, right.bulan, right.tanggal);
        comparison = (leftDate < rightDate) ? -1 : ((rightDate < leftDate) ? 1 : 0);
    } else {
        comparison = (left.id < right.id) ? -1 : ((left.id > right.id) ? 1 : 0);
    }
    return ascending ? comparison < 0 : comparison > 0;
}

std::vector<Buku> sortedBooksWithMethod(const std::vector<Buku> &source,
                                        const QString &field, bool ascending, int method)
{
    std::vector<Buku> result = source;
    const auto less = [&field, ascending](const Buku &a, const Buku &b) {
        return bookLess(a, b, field, ascending);
    };

    if (method == 0) { // Bubble Sort
        for (size_t i = 0; i < result.size(); ++i) {
            bool swapped = false;
            for (size_t j = 0; j + 1 < result.size() - i; ++j) {
                if (less(result[j + 1], result[j])) {
                    std::swap(result[j], result[j + 1]);
                    swapped = true;
                }
            }
            if (!swapped) break;
        }
    } else if (method == 1) { // Selection Sort
        for (size_t i = 0; i < result.size(); ++i) {
            size_t best = i;
            for (size_t j = i + 1; j < result.size(); ++j) {
                if (less(result[j], result[best])) best = j;
            }
            if (best != i) std::swap(result[i], result[best]);
        }
    } else { // Insertion Sort
        for (size_t i = 1; i < result.size(); ++i) {
            Buku key = result[i];
            size_t j = i;
            while (j > 0 && less(key, result[j - 1])) {
                result[j] = result[j - 1];
                --j;
            }
            result[j] = key;
        }
    }

    return result;
}

void fillPreviewList(QListWidget *list, const std::vector<Buku> &books)
{
    list->clear();
    const size_t visibleCount = std::min<size_t>(books.size(), 6);

    if (books.empty()) {
        auto *item = new QListWidgetItem("Belum ada data buku. Tambahkan koleksi terlebih dahulu.");
        item->setTextAlignment(Qt::AlignCenter);
        list->addItem(item);
        return;
    }

    for (size_t i = 0; i < visibleCount; ++i) {
        const Buku &book = books[i];
        QString title = toQString(book.judul).simplified();
        if (title.size() > 27) title = title.left(24) + "...";
        const QString row = QString("%1   %2   ·   %3")
            .arg(static_cast<int>(i + 1), 2, 10, QLatin1Char('0'))
            .arg(title)
            .arg(bookDateText(book));
        list->addItem(row);
    }

    if (books.size() > visibleCount) {
        list->addItem(QString("+ %1 buku lainnya").arg(books.size() - visibleCount));
    }
}

} // namespace

DashboardWindow::DashboardWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("BookSort | Sistem Perpustakaan");
    resize(1180, 760);
    setMinimumSize(960, 650);

    setStyleSheet(R"(
        * { font-family: "Segoe UI", "Arial"; }
        QMainWindow { background: #f3f5f7; }
        QWidget#mainArea { background: #f3f5f7; }
        QWidget#sidebar { background: #233f58; border-right: 3px solid #3c9de0; }
        QLabel#brandIcon { color: white; background: #398dca; border-radius: 7px; font-size: 15px; font-weight: 700; }
        QLabel#brand { color: white; font-size: 17px; font-weight: 700; }
        QLabel#brandCaption { color: #a9c0d2; font-size: 8px; letter-spacing: 1px; }
        QLabel#sidebarCaption { color: #a9c0d2; font-size: 9px; font-weight: 700; letter-spacing: 1px; }
        QPushButton#navButton {
            color: #dce8f1; background: transparent; border: none;
            border-radius: 6px; text-align: left; padding: 9px 10px;
            font-size: 11px; font-weight: 500;
        }
        QPushButton#navButton:hover { background: #315570; color: white; }
        QPushButton#navButton[active="true"] { color: white; background: #398dca; font-weight: 700; }
        QFrame#sidebarHint { background: #2d4c66; border: 1px solid #385c77; border-radius: 8px; }
        QLabel#sidebarHintTitle { color: white; font-size: 10px; font-weight: 700; }
        QLabel#sidebarHintText { color: #bad0df; font-size: 8px; }
        QLabel#sidebarFooter { color: #94aec1; font-size: 8px; }

        QFrame#topBar { background: white; border-bottom: 1px solid #e6ebef; }
        QLabel#breadcrumb { color: #647b8e; font-size: 9px; }
        QLabel#topPill { color: #60778a; background: #f0f4f7; border-radius: 4px; padding: 5px 8px; font-size: 8px; }
        QLabel#adminName { color: #243f58; font-size: 9px; font-weight: 700; }
        QLabel#adminRole { color: #8999a7; font-size: 8px; }

        QLabel#pageTitle { color: #213b54; font-size: 22px; font-weight: 700; }
        QLabel#pageSubtitle { color: #8495a4; font-size: 10px; }
        QLabel#sectionTitle { color: #29445d; font-size: 12px; font-weight: 700; }
        QLabel#fieldLabel { color: #778b9c; font-size: 9px; }
        QLabel#eyebrow { color: #398d70; background: #eaf6f0; border-radius: 4px; padding: 5px 8px; font-size: 8px; font-weight: 700; }
        QLabel#metaLabel { color: #8a9aa8; font-size: 9px; }

        QFrame#surfaceCard, QFrame#statCard, QFrame#previewCard {
            background: white; border: 1px solid #e7edf1; border-radius: 9px;
        }
        QFrame#statCard:hover { border: 1px solid #c7dced; }
        QLabel#statIcon { color: #398dca; background: #eaf3fa; border-radius: 7px; font-size: 13px; font-weight: 700; }
        QLabel#statDot { color: #9fc7df; font-size: 8px; }
        QLabel#statCaption { color: #718699; font-size: 9px; font-weight: 700; letter-spacing: .5px; }
        QLabel#statValue { color: #213b54; font-size: 26px; font-weight: 700; }
        QLabel#statHint { color: #93a1ad; font-size: 8px; }

        QFrame#methodCard { background: white; border: 1px solid #e7edf1; border-radius: 9px; }
        QFrame#methodCard:hover { border: 1px solid #a8cce5; background: #fcfdff; }
        QFrame#methodCard[active="true"] { background: #eef6fc; border: 1px solid #398dca; }
        QLabel#methodIcon { color: #398dca; background: #eaf3fa; border-radius: 6px; font-size: 14px; font-weight: 700; }
        QLabel#methodTitle { color: #29445d; font-size: 11px; font-weight: 700; }
        QLabel#methodDescription { color: #8193a2; font-size: 8px; }
        QLabel#methodHint { color: #398dca; font-size: 8px; }
        QRadioButton#methodRadio { spacing: 0px; }
        QRadioButton#methodRadio::indicator { width: 12px; height: 12px; border: 1px solid #c4d2dd; border-radius: 6px; background: white; }
        QRadioButton#methodRadio::indicator:checked { background: #398dca; border: 3px solid #d9ebf8; }

        QLineEdit, QComboBox, QDateEdit {
            background: white; color: #334e64; border: 1px solid #dce5eb;
            border-radius: 5px; padding: 7px 9px; font-size: 9px; min-height: 16px;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border: 1px solid #398dca; }
        QComboBox::drop-down { border: none; width: 22px; }
        QComboBox QAbstractItemView { background: white; color: #334e64; selection-background-color: #eaf3fa; selection-color: #213b54; border: 1px solid #dce5eb; }
        QPushButton#primaryButton {
            color: white; background: #398dca; border: none;
            border-radius: 5px; padding: 8px 14px; font-size: 9px; font-weight: 700;
        }
        QPushButton#primaryButton:hover { background: #277bb8; }
        QPushButton#primaryButton:pressed { background: #216b9f; padding-top: 9px; }
        QPushButton#secondaryButton {
            color: #3e566b; background: white; border: 1px solid #dce5eb;
            border-radius: 5px; padding: 8px 14px; font-size: 9px; font-weight: 600;
        }
        QPushButton#secondaryButton:hover { background: #f3f8fb; border-color: #b9d3e5; }
        QPushButton#dangerButton {
            color: white; background: #d96868; border: none;
            border-radius: 5px; padding: 8px 14px; font-size: 9px; font-weight: 700;
        }
        QPushButton#dangerButton:hover { background: #c85050; }

        QTableWidget {
            background: white; alternate-background-color: #f8fafb;
            border: 1px solid #e7edf1; border-radius: 8px;
            gridline-color: transparent; color: #3b5368; font-size: 10px;
            selection-background-color: #e8f3fb; selection-color: #213b54;
        }
        QTableWidget::item { padding: 7px; border-bottom: 1px solid #edf1f4; }
        QTableWidget::item:hover { background: #f0f7fc; }
        QHeaderView::section {
            background: #f3f7fa; color: #70879a; border: none;
            border-bottom: 1px solid #e7edf1; padding: 10px 8px; font-size: 9px; font-weight: 700;
        }
        QTableCornerButton::section { background: #f3f7fa; border: none; }
        QTableWidget QScrollBar:vertical { width: 8px; background: transparent; margin: 3px; }
        QTableWidget QScrollBar::handle:vertical { background: #cbd9e3; border-radius: 4px; min-height: 24px; }
        QListWidget { background: transparent; border: none; color: #4d6579; font-size: 9px; outline: none; }
        QListWidget::item { padding: 5px 4px; border-bottom: 1px solid rgba(120,145,164,0.10); }
        QListWidget::item:hover { background: rgba(57,141,202,0.08); }
        QFrame#beforePanel { background: #f4f6f8; border: 1px solid #eef1f3; border-radius: 6px; }
        QFrame#afterPanel { background: #eff7f3; border: 1px solid #e3f0e9; border-radius: 6px; }
        QLabel#previewLabel { color: #8798a6; font-size: 8px; font-weight: 700; }
        QLabel#previewLabelAfter { color: #4c987b; font-size: 8px; font-weight: 700; }
        QLabel#previewArrow { color: #398dca; font-size: 19px; font-weight: 700; }
        QLabel#footerNote { color: #8798a6; font-size: 8px; }
        QScrollBar:vertical { width: 9px; background: transparent; margin: 2px; }
        QScrollBar::handle:vertical { background: #cbd9e3; border-radius: 4px; min-height: 25px; }
        QScrollBar::handle:vertical:hover { background: #a9c0d1; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    )");

    auto *central = new QWidget(this);
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Sidebar inspired by the supplied Figma screen.
    auto *sidebar = new QWidget;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(208);
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(13, 18, 13, 16);
    sideLayout->setSpacing(7);

    auto *brandRow = new QHBoxLayout;
    auto *brandIcon = new QLabel("▣");
    brandIcon->setObjectName("brandIcon");
    brandIcon->setAlignment(Qt::AlignCenter);
    brandIcon->setFixedSize(27, 27);
    auto *brand = new QLabel("BookSort");
    brand->setObjectName("brand");
    brandRow->addWidget(brandIcon);
    brandRow->addWidget(brand);
    brandRow->addStretch();
    sideLayout->addLayout(brandRow);

    auto *brandCaption = new QLabel("KOLEKSI TERATUR, IDE BERTUMBUH");
    brandCaption->setObjectName("brandCaption");
    brandCaption->setContentsMargins(2, 1, 0, 0);
    sideLayout->addWidget(brandCaption);

    auto *menuCaption = new QLabel("MENU UTAMA");
    menuCaption->setObjectName("sidebarCaption");
    menuCaption->setContentsMargins(2, 24, 0, 5);
    sideLayout->addWidget(menuCaption);

    const QStringList navTexts = {
        "▦   Dashboard", "▤   Data Buku", "+   Tambah Buku",
        "▦   Kategori Buku", "↕   Sorting Center", "⌕   Pencarian Buku"
    };
    for (int i = 0; i < navTexts.size(); ++i) {
        auto *button = makeButton(navTexts[i], "navButton");
        button->setMinimumHeight(34);
        button->setProperty("active", i == 0);
        navButtons.push_back(button);
        sideLayout->addWidget(button);
    }
    sideLayout->addStretch(1);

    auto *hint = makeSurface("sidebarHint");
    auto *hintLayout = new QVBoxLayout(hint);
    hintLayout->setContentsMargins(10, 9, 10, 9);
    hintLayout->setSpacing(5);
    auto *hintTitle = new QLabel("Rapi dalam satu tempat.");
    hintTitle->setObjectName("sidebarHintTitle");
    auto *hintText = new QLabel("Kelola, temukan, dan urutkan buku dengan lebih mudah.");
    hintText->setObjectName("sidebarHintText");
    hintText->setWordWrap(true);
    hintLayout->addWidget(hintTitle);
    hintLayout->addWidget(hintText);
    sideLayout->addWidget(hint);

    auto *sidebarFooter = new QLabel("BookSort  /  Library workspace");
    sidebarFooter->setObjectName("sidebarFooter");
    sidebarFooter->setContentsMargins(2, 10, 0, 0);
    sideLayout->addWidget(sidebarFooter);

    // Main area with the slim workspace bar.
    auto *mainArea = new QWidget;
    mainArea->setObjectName("mainArea");
    auto *mainLayout = new QVBoxLayout(mainArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *topBar = makeSurface("topBar");
    topBar->setFixedHeight(48);
    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(22, 0, 22, 0);
    topBreadcrumb = new QLabel("Workspace   /   <b>Dashboard</b>");
    topBreadcrumb->setObjectName("breadcrumb");
    topLayout->addWidget(topBreadcrumb);
    topLayout->addStretch();
    auto *collectionPill = new QLabel("Koleksi pribadi");
    collectionPill->setObjectName("topPill");
    topLayout->addWidget(collectionPill);
    auto *adminBox = new QVBoxLayout;
    adminBox->setSpacing(1);
    auto *adminName = new QLabel("Admin");
    adminName->setObjectName("adminName");
    auto *adminRole = new QLabel("Pustakawan");
    adminRole->setObjectName("adminRole");
    adminBox->addWidget(adminName);
    adminBox->addWidget(adminRole);
    topLayout->addSpacing(18);
    topLayout->addLayout(adminBox);
    mainLayout->addWidget(topBar);

    pages = new QStackedWidget;
    pageOpacity = new QGraphicsOpacityEffect(pages);
    pageOpacity->setOpacity(1.0);
    pages->setGraphicsEffect(pageOpacity);
    mainLayout->addWidget(pages, 1);

    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(mainArea, 1);
    setCentralWidget(central);

    createDashboardPage();
    createBooksPage();
    createCategoriesPage();
    createSortingPage();

    connect(navButtons[0], &QPushButton::clicked, this, [this]() { navigateToPage(0, 0); });
    connect(navButtons[1], &QPushButton::clicked, this, [this]() { navigateToPage(1, 1); });
    connect(navButtons[2], &QPushButton::clicked, this, [this]() {
        setActiveNavigation(2);
        if (topBreadcrumb) topBreadcrumb->setText("Workspace   /   <b>Tambah Buku</b>");
        showBookDialog();
    });
    connect(navButtons[3], &QPushButton::clicked, this, [this]() { navigateToPage(2, 3); });
    connect(navButtons[4], &QPushButton::clicked, this, [this]() { navigateToPage(3, 4); });
    connect(navButtons[5], &QPushButton::clicked, this, [this]() {
        navigateToPage(1, 5);
        if (searchInput) searchInput->setFocus();
    });

    refreshGenreFilter();
    refreshBookTable();
    refreshDashboard();
    refreshCategoryTable();
    refreshSortingPreview(false);
}

void DashboardWindow::setActiveNavigation(int navIndex)
{
    for (int i = 0; i < static_cast<int>(navButtons.size()); ++i) {
        QPushButton *button = navButtons[static_cast<size_t>(i)];
        const bool active = (i == navIndex);
        button->setProperty("active", active);
        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }
}

void DashboardWindow::navigateToPage(int pageIndex, int navIndex)
{
    if (!pages || pageIndex < 0 || pageIndex >= pages->count()) return;
    if (transitionAnimation) return;
    if (navIndex >= 0) {
        setActiveNavigation(navIndex);
        const QString breadcrumbNames[] = {
            "Dashboard", "Data Buku", "Tambah Buku", "Kategori Buku", "Sorting Center", "Pencarian Buku"
        };
        if (topBreadcrumb && navIndex < 6)
            topBreadcrumb->setText("Workspace   /   <b>" + breadcrumbNames[navIndex] + "</b>");
    }
    if (pages->currentIndex() == pageIndex) {
        if (pageIndex == 0) refreshDashboard();
        if (pageIndex == 1) { refreshGenreFilter(); refreshBookTable(); }
        if (pageIndex == 2) refreshCategoryTable();
        if (pageIndex == 3) refreshSortingPreview(false);
        return;
    }

    auto *animation = new QSequentialAnimationGroup(this);
    transitionAnimation = animation;

    auto *fadeOut = new QPropertyAnimation(pageOpacity, "opacity", animation);
    fadeOut->setDuration(110);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.38);
    fadeOut->setEasingCurve(QEasingCurve::InOutCubic);

    auto *fadeIn = new QPropertyAnimation(pageOpacity, "opacity", animation);
    fadeIn->setDuration(190);
    fadeIn->setStartValue(0.38);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::InOutCubic);

    connect(fadeOut, &QPropertyAnimation::finished, this, [this, pageIndex]() {
        pages->setCurrentIndex(pageIndex);
        if (pageIndex == 0) refreshDashboard();
        if (pageIndex == 1) { refreshGenreFilter(); refreshBookTable(); }
        if (pageIndex == 2) refreshCategoryTable();
        if (pageIndex == 3) refreshSortingPreview(false);
    });
    connect(animation, &QSequentialAnimationGroup::finished, this, [this, animation]() {
        pageOpacity->setOpacity(1.0);
        transitionAnimation = nullptr;
        animation->deleteLater();
    });
    animation->start();
}

void DashboardWindow::createDashboardPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(22, 18, 22, 18);
    layout->setSpacing(13);

    auto *headingRow = new QHBoxLayout;
    auto *headingColumn = new QVBoxLayout;
    headingColumn->setSpacing(3);
    auto *title = new QLabel("Dashboard");
    title->setObjectName("pageTitle");
    auto *subtitle = new QLabel("Ringkasan koleksi dan aktivitas perpustakaan.");
    subtitle->setObjectName("pageSubtitle");
    headingColumn->addWidget(title);
    headingColumn->addWidget(subtitle);
    headingRow->addLayout(headingColumn);
    headingRow->addStretch();
    auto *badge = new QLabel("LIBRARY OVERVIEW");
    badge->setObjectName("eyebrow");
    headingRow->addWidget(badge, 0, Qt::AlignTop);
    layout->addLayout(headingRow);

    auto *stats = new QGridLayout;
    stats->setHorizontalSpacing(12);
    stats->setVerticalSpacing(12);
    stats->addWidget(makeStatCard("▤", "Total buku", &totalBooksValue, "Seluruh koleksi terdaftar"), 0, 0);
    stats->addWidget(makeStatCard("▦", "Kategori buku", &totalGenresValue, "Jenis koleksi berbeda"), 0, 1);
    stats->addWidget(makeStatCard("♙", "Jumlah penulis", &totalAuthorsValue, "Penulis dalam katalog"), 0, 2);
    layout->addLayout(stats);

    auto *welcome = makeSurface("surfaceCard");
    auto *welcomeLayout = new QVBoxLayout(welcome);
    welcomeLayout->setContentsMargins(18, 16, 18, 16);
    welcomeLayout->setSpacing(8);
    auto *welcomeTitle = new QLabel("Selamat datang di BookSort");
    welcomeTitle->setObjectName("sectionTitle");
    welcomeTitle->setStyleSheet("font-size: 15px;");
    auto *welcomeText = new QLabel("Atur koleksi buku dengan lebih terstruktur. Kelola data, telusuri kategori, dan lihat bagaimana algoritma sorting menyusun koleksi perpustakaan.");
    welcomeText->setObjectName("pageSubtitle");
    welcomeText->setWordWrap(true);
    auto *quickActions = new QHBoxLayout;
    auto *openBooks = makeButton("Buka Data Buku", "primaryButton");
    auto *openSorting = makeButton("Buka Sorting Center", "secondaryButton");
    auto *addBook = makeButton("+ Tambah Buku", "secondaryButton");
    quickActions->addWidget(openBooks);
    quickActions->addWidget(openSorting);
    quickActions->addWidget(addBook);
    quickActions->addStretch();
    welcomeLayout->addWidget(welcomeTitle);
    welcomeLayout->addWidget(welcomeText);
    welcomeLayout->addSpacing(3);
    welcomeLayout->addLayout(quickActions);
    layout->addWidget(welcome);

    auto *recent = makeSurface("surfaceCard");
    auto *recentLayout = new QVBoxLayout(recent);
    recentLayout->setContentsMargins(18, 14, 18, 14);
    auto *recentTitle = new QLabel("Mulai dari sini");
    recentTitle->setObjectName("sectionTitle");
    auto *recentText = new QLabel("Gunakan Sorting Center untuk membandingkan Bubble Sort, Selection Sort, dan Insertion Sort melalui pratinjau sebelum dan sesudah pengurutan.");
    recentText->setObjectName("pageSubtitle");
    recentText->setWordWrap(true);
    recentLayout->addWidget(recentTitle);
    recentLayout->addWidget(recentText);
    layout->addWidget(recent);
    layout->addStretch(1);

    connect(openBooks, &QPushButton::clicked, this, [this]() { navigateToPage(1, 1); });
    connect(openSorting, &QPushButton::clicked, this, [this]() { navigateToPage(3, 4); });
    connect(addBook, &QPushButton::clicked, this, [this]() { showBookDialog(); });

    pages->addWidget(page);
}

void DashboardWindow::createBooksPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(22, 18, 22, 18);
    layout->setSpacing(11);

    auto *title = new QLabel("Data Buku");
    title->setObjectName("pageTitle");
    auto *subtitle = new QLabel("Kelola seluruh koleksi buku perpustakaan.");
    subtitle->setObjectName("pageSubtitle");
    layout->addWidget(title);
    layout->addWidget(subtitle);

    auto *toolbarCard = makeSurface("surfaceCard");
    auto *toolbar = new QHBoxLayout(toolbarCard);
    toolbar->setContentsMargins(12, 10, 12, 10);
    toolbar->setSpacing(8);

    searchInput = new QLineEdit;
    searchInput->setPlaceholderText("⌕  Cari judul, penulis, atau jenis buku...");
    searchInput->setMinimumWidth(190);
    genreFilter = new QComboBox;
    genreFilter->setMinimumWidth(125);
    sortChoice = new QComboBox;
    sortChoice->addItem("Urutkan: Judul", "judul");
    sortChoice->addItem("Urutkan: Penulis", "penulis");
    sortChoice->addItem("Urutkan: Kategori", "jenis");
    sortChoice->addItem("Urutkan: Tahun", "tahun");
    sortChoice->addItem("Urutkan: ID", "id");
    sortChoice->setMinimumWidth(130);

    auto *sortButton = makeButton("↕  Urutkan", "secondaryButton");
    auto *addButton = makeButton("+  Tambah Buku", "primaryButton");
    toolbar->addWidget(searchInput, 1);
    toolbar->addWidget(genreFilter);
    toolbar->addWidget(sortChoice);
    toolbar->addWidget(sortButton);
    toolbar->addWidget(addButton);
    layout->addWidget(toolbarCard);

    auto *actions = new QHBoxLayout;
    auto *tableCaption = new QLabel("DAFTAR KOLEKSI");
    tableCaption->setObjectName("sectionTitle");
    auto *editButton = makeButton("Edit", "secondaryButton");
    auto *deleteButton = makeButton("Hapus", "dangerButton");
    actions->addWidget(tableCaption);
    actions->addStretch();
    actions->addWidget(editButton);
    actions->addWidget(deleteButton);
    layout->addLayout(actions);

    auto *tableSurface = makeSurface("surfaceCard");
    auto *tableLayout = new QVBoxLayout(tableSurface);
    tableLayout->setContentsMargins(8, 8, 8, 8);
    bookTable = new QTableWidget;
    bookTable->setColumnCount(5);
    bookTable->setHorizontalHeaderLabels({"ID", "Judul Buku", "Jenis", "Penulis", "Tanggal Terbit"});
    bookTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    bookTable->setSelectionMode(QAbstractItemView::SingleSelection);
    bookTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bookTable->setAlternatingRowColors(true);
    bookTable->setShowGrid(false);
    bookTable->verticalHeader()->setVisible(false);
    bookTable->verticalHeader()->setDefaultSectionSize(38);
    bookTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    bookTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    bookTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    bookTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    bookTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tableLayout->addWidget(bookTable);
    layout->addWidget(tableSurface, 1);

    connect(searchInput, &QLineEdit::textChanged, this, [this]() { refreshBookTable(); });
    connect(genreFilter, &QComboBox::currentTextChanged, this, [this]() { refreshBookTable(); });
    connect(sortButton, &QPushButton::clicked, this, [this]() { sortBooks(); });
    connect(addButton, &QPushButton::clicked, this, [this]() { showBookDialog(); });
    connect(editButton, &QPushButton::clicked, this, [this]() { editSelectedBook(); });
    connect(deleteButton, &QPushButton::clicked, this, [this]() { deleteSelectedBook(); });
    connect(bookTable, &QTableWidget::cellDoubleClicked, this, [this](int, int) { editSelectedBook(); });

    pages->addWidget(page);
}

void DashboardWindow::createCategoriesPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(22, 18, 22, 18);
    layout->setSpacing(12);

    auto *title = new QLabel("Kategori Buku");
    title->setObjectName("pageTitle");
    auto *subtitle = new QLabel("Lihat persebaran koleksi berdasarkan jenis buku.");
    subtitle->setObjectName("pageSubtitle");
    layout->addWidget(title);
    layout->addWidget(subtitle);

    auto *caption = new QLabel("RINGKASAN KATEGORI");
    caption->setObjectName("sectionTitle");
    layout->addWidget(caption);

    auto *surface = makeSurface("surfaceCard");
    auto *surfaceLayout = new QVBoxLayout(surface);
    surfaceLayout->setContentsMargins(8, 8, 8, 8);
    categoryTable = new QTableWidget;
    categoryTable->setColumnCount(2);
    categoryTable->setHorizontalHeaderLabels({"Kategori", "Jumlah Buku"});
    categoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    categoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    categoryTable->setShowGrid(false);
    categoryTable->verticalHeader()->setVisible(false);
    categoryTable->verticalHeader()->setDefaultSectionSize(40);
    categoryTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    categoryTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    surfaceLayout->addWidget(categoryTable);
    layout->addWidget(surface, 1);

    pages->addWidget(page);
}

void DashboardWindow::createSortingPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(22, 16, 22, 14);
    layout->setSpacing(10);

    auto *headingRow = new QHBoxLayout;
    auto *headingColumn = new QVBoxLayout;
    headingColumn->setSpacing(2);
    auto *title = new QLabel("Sorting Center");
    title->setObjectName("pageTitle");
    auto *subtitle = new QLabel("Pilih metode pengurutan koleksi buku.");
    subtitle->setObjectName("pageSubtitle");
    headingColumn->addWidget(title);
    headingColumn->addWidget(subtitle);
    headingRow->addLayout(headingColumn);
    headingRow->addStretch();
    auto *eyebrow = new QLabel("INTI BOOKSORT");
    eyebrow->setObjectName("eyebrow");
    headingRow->addWidget(eyebrow, 0, Qt::AlignTop);
    layout->addLayout(headingRow);

    methodGroup = new QButtonGroup(this);
    methodGroup->setExclusive(true);
    auto *methods = new QHBoxLayout;
    methods->setSpacing(10);
    QRadioButton *bubbleRadio = nullptr;
    QRadioButton *selectionRadio = nullptr;
    QRadioButton *insertionRadio = nullptr;
    methods->addWidget(makeMethodCard("↕", "Bubble Sort",
        "Membandingkan buku bersebelahan untuk membentuk urutan.", "Cocok untuk belajar langkah demi langkah", methodGroup, 0, &bubbleRadio), 1);
    methods->addWidget(makeMethodCard("▤", "Selection Sort",
        "Memilih elemen terbaik lalu menempatkannya di posisi tepat.", "Memilih elemen satu per satu", methodGroup, 1, &selectionRadio), 1);
    methods->addWidget(makeMethodCard("▣", "Insertion Sort",
        "Menyisipkan setiap buku ke posisi yang sesuai dalam urutan.", "Efisien untuk data yang hampir terurut", methodGroup, 2, &insertionRadio), 1);
    insertionRadio->setChecked(true);
    layout->addLayout(methods);

    auto *configCard = makeSurface("surfaceCard");
    auto *configLayout = new QVBoxLayout(configCard);
    configLayout->setContentsMargins(14, 12, 14, 12);
    configLayout->setSpacing(9);
    auto *configTitle = new QLabel("Konfigurasi pengurutan");
    configTitle->setObjectName("sectionTitle");
    configLayout->addWidget(configTitle);

    auto *formRow = new QHBoxLayout;
    formRow->setSpacing(9);
    auto *fieldColumn = new QVBoxLayout;
    fieldColumn->setSpacing(4);
    fieldColumn->addWidget(makeFieldLabel("Kolom pengurutan"));
    sortingField = new QComboBox;
    sortingField->addItem("Tanggal terbit", "tanggal");
    sortingField->addItem("Judul buku", "judul");
    sortingField->addItem("Penulis", "penulis");
    sortingField->addItem("Kategori", "jenis");
    sortingField->addItem("ID buku", "id");
    fieldColumn->addWidget(sortingField);

    auto *orderColumn = new QVBoxLayout;
    orderColumn->setSpacing(4);
    orderColumn->addWidget(makeFieldLabel("Urutan"));
    sortingOrder = new QComboBox;
    sortingOrder->addItem("Naik · A–Z / lama → baru", "asc");
    sortingOrder->addItem("Turun · Z–A / baru → lama", "desc");
    orderColumn->addWidget(sortingOrder);

    auto *runButton = makeButton("▶  Atur Ulang", "primaryButton");
    runButton->setMinimumWidth(150);
    runButton->setMinimumHeight(36);
    auto *resetButton = makeButton("Reset", "secondaryButton");
    resetButton->setMinimumWidth(90);
    resetButton->setMinimumHeight(36);

    formRow->addLayout(fieldColumn, 1);
    formRow->addLayout(orderColumn, 1);
    formRow->addSpacing(4);
    formRow->addWidget(runButton, 1, Qt::AlignBottom);
    formRow->addWidget(resetButton, 0, Qt::AlignBottom);
    configLayout->addLayout(formRow);

    auto *configHint = new QLabel("Metode aktif: Insertion Sort");
    configHint->setObjectName("metaLabel");
    configLayout->addWidget(configHint);
    layout->addWidget(configCard);

    auto *previewCard = makeSurface("previewCard");
    auto *previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setContentsMargins(14, 12, 14, 12);
    previewLayout->setSpacing(8);

    auto *previewHeader = new QHBoxLayout;
    auto *previewTitle = new QLabel("Pratinjau hasil");
    previewTitle->setObjectName("sectionTitle");
    sortingMethodBadge = new QLabel("Simulasi");
    sortingMethodBadge->setObjectName("eyebrow");
    previewHeader->addWidget(previewTitle);
    previewHeader->addSpacing(8);
    previewHeader->addWidget(sortingMethodBadge);
    previewHeader->addStretch();
    auto *exampleTag = new QLabel("DATA AKTUAL");
    exampleTag->setObjectName("eyebrow");
    previewHeader->addWidget(exampleTag);
    previewLayout->addLayout(previewHeader);

    sortingMeta = new QLabel("Data buku akan ditampilkan di sini setelah pengurutan dijalankan.");
    sortingMeta->setObjectName("metaLabel");
    previewLayout->addWidget(sortingMeta);

    auto *previewRow = new QHBoxLayout;
    previewRow->setSpacing(9);
    auto *beforePanel = makeSurface("beforePanel");
    auto *beforeLayout = new QVBoxLayout(beforePanel);
    beforeLayout->setContentsMargins(10, 8, 10, 8);
    beforeLayout->setSpacing(4);
    auto *beforeLabel = new QLabel("SEBELUM");
    beforeLabel->setObjectName("previewLabel");
    beforePreview = new QListWidget;
    beforePreview->setFocusPolicy(Qt::NoFocus);
    beforePreview->setSelectionMode(QAbstractItemView::NoSelection);
    beforeLayout->addWidget(beforeLabel);
    beforeLayout->addWidget(beforePreview, 1);

    auto *arrow = new QLabel("→");
    arrow->setObjectName("previewArrow");
    arrow->setAlignment(Qt::AlignCenter);

    auto *afterPanel = makeSurface("afterPanel");
    auto *afterLayout = new QVBoxLayout(afterPanel);
    afterLayout->setContentsMargins(10, 8, 10, 8);
    afterLayout->setSpacing(4);
    auto *afterLabel = new QLabel("SESUDAH PENGURUTAN");
    afterLabel->setObjectName("previewLabelAfter");
    afterPreview = new QListWidget;
    afterPreview->setFocusPolicy(Qt::NoFocus);
    afterPreview->setSelectionMode(QAbstractItemView::NoSelection);
    afterLayout->addWidget(afterLabel);
    afterLayout->addWidget(afterPreview, 1);

    previewRow->addWidget(beforePanel, 1);
    previewRow->addWidget(arrow, 0);
    previewRow->addWidget(afterPanel, 1);
    previewLayout->addLayout(previewRow, 1);
    layout->addWidget(previewCard, 1);

    auto *footer = new QHBoxLayout;
    auto *footerIcon = new QLabel("‹›");
    footerIcon->setStyleSheet("color: #7390a5; font-size: 12px; font-weight: 700;");
    auto *footerText = new QLabel("Pratinjau menggunakan data koleksi saat ini. Urutan pada data utama tidak diubah.");
    footerText->setObjectName("footerNote");
    footer->addWidget(footerIcon);
    footer->addWidget(footerText);
    footer->addStretch();
    layout->addLayout(footer);

    connect(bubbleRadio, &QRadioButton::toggled, this, [configHint](bool checked) {
        if (checked) configHint->setText("Metode aktif: Bubble Sort");
    });
    connect(selectionRadio, &QRadioButton::toggled, this, [configHint](bool checked) {
        if (checked) configHint->setText("Metode aktif: Selection Sort");
    });
    connect(insertionRadio, &QRadioButton::toggled, this, [configHint](bool checked) {
        if (checked) configHint->setText("Metode aktif: Insertion Sort");
    });
    connect(runButton, &QPushButton::clicked, this, [this]() { runSortingPreview(); });
    connect(resetButton, &QPushButton::clicked, this, [this]() { resetSortingPreview(); });
    connect(sortingField, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        if (sortingMeta) sortingMeta->setText("Pengaturan berubah. Tekan Atur Ulang untuk memperbarui pratinjau.");
    });
    connect(sortingOrder, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        if (sortingMeta) sortingMeta->setText("Pengaturan berubah. Tekan Atur Ulang untuk memperbarui pratinjau.");
    });

    pages->addWidget(page);
}

void DashboardWindow::refreshDashboard()
{
    std::set<std::string> genres;
    std::set<std::string> authors;
    for (const Buku &book : daftarBuku) {
        if (!book.jenis.empty()) genres.insert(book.jenis);
        if (!book.penulis.empty()) authors.insert(book.penulis);
    }

    if (totalBooksValue) totalBooksValue->setText(QString::number(static_cast<qulonglong>(daftarBuku.size())));
    if (totalGenresValue) totalGenresValue->setText(QString::number(static_cast<qulonglong>(genres.size())));
    if (totalAuthorsValue) totalAuthorsValue->setText(QString::number(static_cast<qulonglong>(authors.size())));
}

void DashboardWindow::refreshGenreFilter()
{
    if (!genreFilter) return;

    const QString previous = genreFilter->currentText();
    const QSignalBlocker blocker(genreFilter);
    genreFilter->clear();
    genreFilter->addItem("Semua Jenis");

    std::set<std::string> genres;
    for (const Buku &book : daftarBuku) {
        if (!book.jenis.empty()) genres.insert(book.jenis);
    }
    for (const std::string &genre : genres) genreFilter->addItem(toQString(genre));

    const int index = genreFilter->findText(previous);
    genreFilter->setCurrentIndex(index >= 0 ? index : 0);
}

void DashboardWindow::refreshBookTable()
{
    if (!bookTable) return;

    const QString query = searchInput ? searchInput->text().trimmed() : QString();
    const QString selectedGenre = genreFilter ? genreFilter->currentText() : "Semua Jenis";
    bookTable->clearSpans();
    bookTable->setRowCount(0);

    for (const Buku &book : daftarBuku) {
        const QString title = toQString(book.judul);
        const QString genre = toQString(book.jenis);
        const QString author = toQString(book.penulis);
        const bool matchesSearch = query.isEmpty()
            || title.contains(query, Qt::CaseInsensitive)
            || genre.contains(query, Qt::CaseInsensitive)
            || author.contains(query, Qt::CaseInsensitive);
        const bool matchesGenre = selectedGenre == "Semua Jenis" || genre == selectedGenre;
        if (!matchesSearch || !matchesGenre) continue;

        const int row = bookTable->rowCount();
        bookTable->insertRow(row);
        bookTable->setItem(row, 0, new QTableWidgetItem(QString::number(book.id)));
        bookTable->setItem(row, 1, new QTableWidgetItem(title));
        bookTable->setItem(row, 2, new QTableWidgetItem(genre));
        bookTable->setItem(row, 3, new QTableWidgetItem(author));
        bookTable->setItem(row, 4, new QTableWidgetItem(bookDateText(book)));
    }

    if (bookTable->rowCount() == 0) {
        bookTable->setRowCount(1);
        auto *empty = new QTableWidgetItem("Belum ada data yang cocok. Coba ubah pencarian atau tambahkan buku baru.");
        empty->setTextAlignment(Qt::AlignCenter);
        bookTable->setItem(0, 0, empty);
        bookTable->setSpan(0, 0, 1, 5);
    }
}

void DashboardWindow::refreshCategoryTable()
{
    if (!categoryTable) return;

    std::map<std::string, int> counts;
    for (const Buku &book : daftarBuku) {
        const std::string key = book.jenis.empty() ? "Tanpa kategori" : book.jenis;
        ++counts[key];
    }

    categoryTable->clearSpans();
    categoryTable->setRowCount(0);
    for (const auto &entry : counts) {
        const int row = categoryTable->rowCount();
        categoryTable->insertRow(row);
        categoryTable->setItem(row, 0, new QTableWidgetItem(toQString(entry.first)));
        categoryTable->setItem(row, 1, new QTableWidgetItem(QString::number(entry.second)));
        categoryTable->item(row, 1)->setTextAlignment(Qt::AlignCenter);
    }
    if (categoryTable->rowCount() == 0) {
        categoryTable->setRowCount(1);
        auto *empty = new QTableWidgetItem("Belum ada kategori. Tambahkan buku untuk mulai membuat kategori.");
        empty->setTextAlignment(Qt::AlignCenter);
        categoryTable->setItem(0, 0, empty);
        categoryTable->setSpan(0, 0, 1, 2);
    }
}

void DashboardWindow::refreshSortingPreview(bool runAlgorithm)
{
    if (!beforePreview || !afterPreview) return;
    const std::vector<Buku> currentBooks(daftarBuku.begin(), daftarBuku.end());
    fillPreviewList(beforePreview, currentBooks);

    if (runAlgorithm) {
        const QString field = sortingField ? sortingField->currentData().toString() : "tanggal";
        const bool ascending = !sortingOrder || sortingOrder->currentData().toString() != "desc";
        const int method = methodGroup ? std::max(0, methodGroup->checkedId()) : 2;
        const std::vector<Buku> sorted = sortedBooksWithMethod(currentBooks, field, ascending, method);
        fillPreviewList(afterPreview, sorted);

        const QString methodNames[] = {"Bubble Sort", "Selection Sort", "Insertion Sort"};
        const QString fieldNames[] = {"Judul buku", "Penulis", "Kategori", "Tanggal terbit", "ID buku"};
        const QString fieldKeys[] = {"judul", "penulis", "jenis", "tanggal", "id"};
        QString fieldName = "Tanggal terbit";
        for (int i = 0; i < 5; ++i) {
            if (field == fieldKeys[i]) fieldName = fieldNames[i];
        }
        if (sortingMeta) {
            sortingMeta->setText(QString("%1 buku diproses  •  %2 berdasarkan %3")
                .arg(currentBooks.size()).arg(methodNames[std::clamp(method, 0, 2)], fieldName));
        }
        if (sortingMethodBadge) sortingMethodBadge->setText("Selesai dipratinjau");
    } else {
        fillPreviewList(afterPreview, currentBooks);
        if (sortingMeta) {
            sortingMeta->setText(currentBooks.empty()
                ? "Belum ada data buku. Tambahkan buku untuk mencoba algoritma sorting."
                : QString("%1 buku tersedia  •  Atur metode dan jalankan pratinjau.").arg(currentBooks.size()));
        }
        if (sortingMethodBadge) sortingMethodBadge->setText("Simulasi");
    }
}

void DashboardWindow::sortBooks()
{
    if (!sortChoice) return;
    const QString key = sortChoice->currentData().toString();
    std::sort(daftarBuku.begin(), daftarBuku.end(), [&key](const Buku &a, const Buku &b) {
        if (key == "judul") return QString::compare(toQString(a.judul), toQString(b.judul), Qt::CaseInsensitive) < 0;
        if (key == "penulis") return QString::compare(toQString(a.penulis), toQString(b.penulis), Qt::CaseInsensitive) < 0;
        if (key == "jenis") return QString::compare(toQString(a.jenis), toQString(b.jenis), Qt::CaseInsensitive) < 0;
        if (key == "tahun") return std::make_tuple(a.tahun, a.bulan, a.tanggal) < std::make_tuple(b.tahun, b.bulan, b.tanggal);
        return a.id < b.id;
    });
    refreshBookTable();
    refreshDashboard();
    refreshCategoryTable();
}

void DashboardWindow::runSortingPreview()
{
    if (!methodGroup || methodGroup->checkedId() < 0) return;
    refreshSortingPreview(true);
}

void DashboardWindow::resetSortingPreview()
{
    if (methodGroup && methodGroup->button(2)) methodGroup->button(2)->setChecked(true);
    if (sortingField) sortingField->setCurrentIndex(0);
    if (sortingOrder) sortingOrder->setCurrentIndex(0);
    refreshSortingPreview(false);
}

void DashboardWindow::showBookDialog(Buku *book)
{
    QDialog dialog(this);
    dialog.setWindowTitle(book ? "Edit Data Buku" : "Tambah Buku Baru");
    dialog.setMinimumWidth(420);
    dialog.setStyleSheet(styleSheet());

    auto *form = new QFormLayout(&dialog);
    form->setContentsMargins(22, 22, 22, 18);
    form->setSpacing(12);

    auto *dialogTitle = new QLabel(book ? "Perbarui informasi buku" : "Tambahkan buku ke koleksi");
    dialogTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #213b54;");
    form->addRow(dialogTitle);

    auto *titleInput = new QLineEdit;
    auto *genreInput = new QLineEdit;
    auto *authorInput = new QLineEdit;
    auto *dateInput = new QDateEdit;
    dateInput->setCalendarPopup(true);
    dateInput->setDisplayFormat("dd/MM/yyyy");
    dateInput->setDate(QDate::currentDate());
    titleInput->setPlaceholderText("Masukkan judul buku");
    genreInput->setPlaceholderText("Contoh: Fiksi, Sejarah, Pendidikan");
    authorInput->setPlaceholderText("Masukkan nama penulis");

    if (book) {
        titleInput->setText(toQString(book->judul));
        genreInput->setText(toQString(book->jenis));
        authorInput->setText(toQString(book->penulis));
        const QDate savedDate(book->tahun, book->bulan, book->tanggal);
        if (savedDate.isValid()) dateInput->setDate(savedDate);
    }

    form->addRow("Judul Buku", titleInput);
    form->addRow("Jenis Buku", genreInput);
    form->addRow("Penulis", authorInput);
    form->addRow("Tanggal Terbit", dateInput);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText("Simpan");
    buttons->button(QDialogButtonBox::Cancel)->setText("Batal");
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    const QString title = titleInput->text().trimmed();
    const QString genre = genreInput->text().trimmed();
    const QString author = authorInput->text().trimmed();
    if (title.isEmpty() || genre.isEmpty() || author.isEmpty()) {
        QMessageBox::warning(this, "Data belum lengkap", "Judul, jenis buku, dan penulis wajib diisi.");
        return;
    }

    const QDate date = dateInput->date();
    if (book) {
        book->judul = toStdString(title);
        book->jenis = toStdString(genre);
        book->penulis = toStdString(author);
        book->tanggal = date.day();
        book->bulan = date.month();
        book->tahun = date.year();
    } else {
        int newId = (nextID > 0) ? nextID : 1;
        for (const Buku &existing : daftarBuku) newId = std::max(newId, existing.id + 1);

        Buku newBook{};
        newBook.id = newId;
        newBook.judul = toStdString(title);
        newBook.jenis = toStdString(genre);
        newBook.penulis = toStdString(author);
        newBook.tanggal = date.day();
        newBook.bulan = date.month();
        newBook.tahun = date.year();
        daftarBuku.push_back(newBook);
        nextID = newId + 1;
    }

    refreshGenreFilter();
    refreshBookTable();
    refreshDashboard();
    refreshCategoryTable();
    refreshSortingPreview(false);
}

void DashboardWindow::editSelectedBook()
{
    const int row = bookTable ? bookTable->currentRow() : -1;
    if (row < 0 || !bookTable->item(row, 0) || bookTable->columnSpan(row, 0) > 1) {
        QMessageBox::information(this, "Pilih buku", "Pilih satu baris buku yang ingin diedit.");
        return;
    }

    bool ok = false;
    const int id = bookTable->item(row, 0)->text().toInt(&ok);
    if (!ok) return;
    auto it = std::find_if(daftarBuku.begin(), daftarBuku.end(), [id](const Buku &book) { return book.id == id; });
    if (it == daftarBuku.end()) {
        QMessageBox::warning(this, "Data tidak ditemukan", "Buku yang dipilih tidak ditemukan.");
        return;
    }
    showBookDialog(&(*it));
}

void DashboardWindow::deleteSelectedBook()
{
    const int row = bookTable ? bookTable->currentRow() : -1;
    if (row < 0 || !bookTable->item(row, 0) || bookTable->columnSpan(row, 0) > 1) {
        QMessageBox::information(this, "Pilih buku", "Pilih satu baris buku yang ingin dihapus.");
        return;
    }

    bool ok = false;
    const int id = bookTable->item(row, 0)->text().toInt(&ok);
    if (!ok) return;

    const auto answer = QMessageBox::question(
        this, "Konfirmasi Hapus", "Yakin ingin menghapus buku yang dipilih?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    const auto oldSize = daftarBuku.size();
    daftarBuku.erase(
        std::remove_if(daftarBuku.begin(), daftarBuku.end(), [id](const Buku &book) { return book.id == id; }),
        daftarBuku.end());

    if (daftarBuku.size() != oldSize) {
        refreshGenreFilter();
        refreshBookTable();
        refreshDashboard();
        refreshCategoryTable();
        refreshSortingPreview(false);
    }
}
