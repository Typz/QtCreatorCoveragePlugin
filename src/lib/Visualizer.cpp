#include "Visualizer.h"

#include "Mark.h"
#include "MarkManager.h"
#include "Node.h"
#include "FileNode.h"
#include "ProjectTreeManager.h"
#include "LinePaintHandler.h"
#include "codecoverageconstants.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>

#include <QPlainTextEdit>
#include <QScrollBar>

#include <QAction>
#include <QDebug>

Visualizer::Visualizer(ProjectTreeManager *projectTreeManager, QAction *renderAction, QObject *parent) :
    QObject(parent),
    projectTreeManager(projectTreeManager),
    markManager(new MarkManager(this)),
    renderAction(renderAction)
{
    using namespace Core;
    EditorManager *editorManager = EditorManager::instance();

    connect(editorManager,SIGNAL(currentEditorChanged(Core::IEditor*)),SLOT(repaintMarks()),Qt::QueuedConnection);
    connect(editorManager,SIGNAL(currentEditorChanged(Core::IEditor*)),SLOT(bindEditorToPainting(Core::IEditor*)));

    connect(renderAction,SIGNAL(triggered(bool)),SLOT(repaintMarks()));
    connect(projectTreeManager,SIGNAL(changed()),SLOT(repaintMarks()));
}

void Visualizer::refreshMarks()
{
    markManager->removeAllMarks();

    Node *rootNode = projectTreeManager->getRootNode();
    if (!rootNode)
        return;

    using namespace Core;
    foreach (IEditor * editor, EditorManager::visibleEditors()) {
        const QString fileName = editor->document()->filePath().toString();
        QList<Node *> leafs = rootNode->findLeafs(fileName.split(QLatin1Char('/')).last());
        foreach (Node *leaf, leafs)
            if (FileNode *fileNode = dynamic_cast<FileNode *>(leaf))
                if (fileNode->getFullAbsoluteName() == fileName) {
                    foreach (const LineHit &lineHit, fileNode->getLineHitList())
                        markManager->addMark(fileName, lineHit.pos, lineHit.hit);
                }
    }
}

void Visualizer::renderCoverage()
{
    using namespace Core;

    EditorManager *editorManager = EditorManager::instance();
    IEditor *currentEditor = editorManager->currentEditor();
    if (!currentEditor)
        return;

    TextEditor::TextEditorWidget *textEdit = qobject_cast<TextEditor::TextEditorWidget *>(currentEditor->widget());
    if (!textEdit)
        return;

    if (renderAction->isChecked()) {
        renderCoverage(textEdit);
    } else {
        clearCoverage(textEdit);
    }
}

void Visualizer::renderCoverage(TextEditor::TextEditorWidget *textEdit) const
{
    LinePaintHandler painter(textEdit, getLineCoverage());
    painter.render();
}

void Visualizer::clearCoverage(TextEditor::TextEditorWidget *textEdit) const
{
    LinePaintHandler cleaner(textEdit, QMap<int, int>());
    cleaner.clear();
}

void Visualizer::bindEditorToPainting(Core::IEditor *editor)
{
    if (!editor)
        return;

    if (QPlainTextEdit *plainTextEdit = qobject_cast<QPlainTextEdit *>(editor->widget())) {
        connect(plainTextEdit,SIGNAL(cursorPositionChanged()),SLOT(renderCoverage()), Qt::UniqueConnection);
        connect(plainTextEdit->verticalScrollBar(),SIGNAL(valueChanged(int)),SLOT(renderCoverage()),Qt::UniqueConnection);
    }
}

void Visualizer::repaintMarks()
{
    if (!renderAction->isChecked()) {
        markManager->removeAllMarks();
    } else {
        refreshMarks();
    }

    renderCoverage();
}

TextEditor::BaseTextEditor *Visualizer::currentTextEditor() const
{
    Core::EditorManager *em = Core::EditorManager::instance();
    Core::IEditor *currEditor = em->currentEditor();
    if (!currEditor)
        return 0;
    return qobject_cast<TextEditor::BaseTextEditor *>(currEditor);
}

QMap<int, int> Visualizer::getLineCoverage() const
{
    using namespace TextEditor;
    QMap<int, int> lineCoverage;

    if (BaseTextEditor *textEditor = currentTextEditor())
        if (TextDocument *textDocument = textEditor->textDocument())
            foreach (TextMark *mark, textDocument->marks())
                if (mark->category() == COVERAGE_TEXT_MARK_CATEGORY) {
                    Mark *trueMark = static_cast<Mark *>(mark);
                    lineCoverage.insert(trueMark->lineNumber(), trueMark->getType());
                }

    return lineCoverage;
}
