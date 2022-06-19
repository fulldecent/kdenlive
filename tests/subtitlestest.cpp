#include "catch.hpp"
#include "test_utils.hpp"

#include "definitions.h"
#define private public
#define protected public
#include "core.h"
#include "doc/docundostack.hpp"
#include "doc/kdenlivedoc.h"

using namespace fakeit;
Mlt::Profile profile_subs;

TEST_CASE("Read subtitle file", "[Subtitles]")
{
    // Create timeline
    auto binModel = pCore->projectItemModel();
    binModel->clean();
    std::shared_ptr<DocUndoStack> undoStack = std::make_shared<DocUndoStack>(nullptr);
    std::shared_ptr<MarkerListModel> guideModel = std::make_shared<MarkerListModel>(undoStack);

    // Here we do some trickery to enable testing.
    // We mock the project class so that the undoStack function returns our undoStack
    Mock<KdenliveDoc> docMock;
    When(Method(docMock, getDocumentProperty)).AlwaysDo([](const QString &name, const QString &defaultValue) {
        Q_UNUSED(name)
        Q_UNUSED(defaultValue)
        qDebug() << "Intercepted call";
        return QStringLiteral("");
    });
    KdenliveDoc &mockedDoc = docMock.get();

    // We mock the project class so that the undoStack function returns our undoStack, and our mocked document
    Mock<ProjectManager> pmMock;
    When(Method(pmMock, undoStack)).AlwaysReturn(undoStack);
    When(Method(pmMock, cacheDir)).AlwaysReturn(QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));
    When(Method(pmMock, current)).AlwaysReturn(&mockedDoc);

    ProjectManager &mocked = pmMock.get();
    pCore->m_projectManager = &mocked;
    pCore->m_projectManager->m_project = &mockedDoc;
    pCore->m_projectManager->m_project->m_guideModel = guideModel;

    // We also mock timeline object to spy few functions and mock others
    TimelineItemModel tim(&profile_subs, undoStack);
    Mock<TimelineItemModel> timMock(tim);
    auto timeline = std::shared_ptr<TimelineItemModel>(&timMock.get(), [](...) {});
    TimelineItemModel::finishConstruct(timeline, guideModel);

    // Create a request
    int tid1 = TrackModel::construct(timeline, -1, -1, QString(), true);
    int tid3 = TrackModel::construct(timeline, -1, -1, QString(), true);
    int tid2 = TrackModel::construct(timeline);
    int tid4 = TrackModel::construct(timeline);

    // Initialize subtitle model
    std::shared_ptr<SubtitleModel> subtitleModel(new SubtitleModel(timeline->tractor(), timeline));
    timeline->setSubModel(subtitleModel);
    mockedDoc.initializeSubtitles(subtitleModel);

    SECTION("Load a subtitle file")
    {
        QString subtitleFile = sourcesPath + "/dataset/01.srt";
        subtitleModel->importSubtitle(subtitleFile);
        // Ensure the 3 dialogues are loaded
        REQUIRE(subtitleModel->rowCount() == 3);
        QList<SubtitledTime> allSubs = subtitleModel->getAllSubtitles();
        QList<GenTime> sTime;
        QList<GenTime> controleTime;
        controleTime << GenTime(140, 25) << GenTime(265, 25) << GenTime(503, 25) << GenTime(628, 25) << GenTime(628, 25) << GenTime(875, 25);
        QStringList subtitlesText;
        QStringList control = {QStringLiteral("J'hésite à vérifier"), QStringLiteral("Ce test de sous-titres"), QStringLiteral("!! Quand même !!")};
        for (const auto &s : allSubs) {
            subtitlesText << s.subtitle();
            sTime << s.start();
            sTime << s.end();
        }
        // Ensure the texts are read correctly
        REQUIRE(subtitlesText == control);
        // Ensure timeing is correct
        REQUIRE(sTime == controleTime);
    }
    binModel->clean();
    pCore->m_projectManager = nullptr;
}