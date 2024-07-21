#include "vymmodelwrapper.h"

#include <QMessageBox>

#include "attributeitem.h"
#include "attribute-wrapper.h"
#include "branchitem.h"
#include "branch-container.h"
#include "branch-wrapper.h"
#include "foo-wrapper.h"
#include "imageitem.h"
#include "image-wrapper.h"
#include "misc.h"
#include "scripting.h"
#include "vymmodel.h"
#include "vymtext.h"
#include "xlink.h"
#include "xlinkitem.h"
#include "xmlobj.h" // include quoteQuotes

#include <QJSEngine>
extern QJSEngine *scriptEngine;

///////////////////////////////////////////////////////////////////////////
VymModelWrapper::VymModelWrapper(VymModel *m) { model = m; }

void VymModelWrapper::addMapCenterAtPos(qreal x, qreal y)
{
    if (!model->addMapCenterAtPos(QPointF(x, y)))
        scriptEngine->throwError(
                QJSValue::GenericError,
                "Couldn't add mapcenter");
}

void VymModelWrapper::addSlide() { model->addSlide(); }

int VymModelWrapper::centerCount()
{
    int r = model->centerCount();
    return setResult(r);
}

void VymModelWrapper::centerOnID(const QString &id)
{
    if (!model->centerOnID(id))
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Could not center on ID %1").arg(id));
}

void VymModelWrapper::copy() { model->copy(); }

void VymModelWrapper::cut() { model->cut(); }

int VymModelWrapper::depth()
{
    TreeItem *selti = model->getSelectedItem();
    if (selti)
        return setResult(selti->depth());

    scriptEngine->throwError(
            QJSValue::GenericError,
            "Nothing selected");
    return -1;
}

void VymModelWrapper::detach()
{
    model->detach();
}

bool VymModelWrapper::exportMap(QJSValueList args)
{
    int argumentsCount = args.count();

    bool r = false;

    if (argumentsCount == 0) {
        scriptEngine->throwError(
                QJSValue::GenericError,
                "Not enough arguments");
        return setResult(r);
    }

    QString format;
    format = args[0].toString();

    if (argumentsCount == 1) {
        if (format == "Last") {
            model->exportLast();
            r = true;
        }
        else
            scriptEngine->throwError(
                    QJSValue::GenericError,
                    "Filename missing");
        return setResult(r);
    }

    QString filename;

    filename = args[1].toString();

    if (format == "AO") {
        model->exportAO(filename, false);
    }
    else if (format == "ASCII") {
        bool listTasks = false;
        if (argumentsCount == 3 && args[2].toString() == "true")
            listTasks = true;
        model->exportASCII(filename, listTasks, false);
    }
    else if (format == "ConfluenceNewPage") {
        // 0: General export format
        // 1: URL of parent page (required)
        // 2: page title (required)
        if (argumentsCount < 3) {
            scriptEngine->throwError(
                    QJSValue::GenericError,
                    QString("Confluence export new page: Only %1 instead of 3 parameters given")
                     .arg(argumentsCount));
            return setResult(r);
        }

        QString url = args[2].toString();
        QString pageName = args[3].toString();

        model->exportConfluence(true, url, pageName, false);
    }
    else if (format == "ConfluenceUpdatePage") {
        // 0: General export format
        // 1: URL of  page to be updated
        // 2: page title (optional)
        if (argumentsCount == 1) {
            scriptEngine->throwError(
                    QJSValue::GenericError,
                    "URL of new page missing in Confluence export");
            return setResult(r);
        }
        QString url = args[1].toString();

        QString title = "";
        if (argumentsCount == 3) {
            title = args[2].toString();
        }

        model->exportConfluence(false, url, title, false);
    }
    else if (format == "CSV") {
        model->exportCSV(filename, false);
    }
    else if (format == "HTML") {
        if (argumentsCount < 3) {
            scriptEngine->throwError(
                    QJSValue::GenericError,
                    "Path missing in HTML export");
            return setResult(r);
        }
        QString dpath = args[2].toString();
        model->exportHTML(filename, dpath, false);
    }
    else if (format == "Image") {
        QString imgFormat;
        if (argumentsCount == 2)
            imgFormat = "PNG";
        else if (argumentsCount == 3)
            imgFormat = args[2].toString();

        QStringList formats;
        formats << "PNG";
        formats << "GIF";
        formats << "JPG";
        formats << "JPEG", formats << "PNG", formats << "PBM", formats << "PGM",
            formats << "PPM", formats << "TIFF", formats << "XBM",
            formats << "XPM";
        if (formats.indexOf(imgFormat) < 0) {
            scriptEngine->throwError(
                    QJSValue::GenericError,
                    QString("%1 not one of the known export formats: ")
                         .arg(imgFormat)
                         .arg(formats.join(",")));
            return setResult(r);
        }
        model->exportImage(filename, false, imgFormat);
    }
    else if (format == "Impress") {
        if (argumentsCount < 3) {
            scriptEngine->throwError(
                    QJSValue::GenericError,
                    "Template file  missing in export to Impress");
            return setResult(r);
        }
        QString templ = args[2].toString();
        model->exportImpress(filename, templ);
    }
    else if (format == "LaTeX") {
        model->exportLaTeX(filename, false);
    }
    else if (format == "Markdown") {
        model->exportMarkdown(filename, false);
    }    
    else if (format == "OrgMode") {
        model->exportOrgMode(filename, false);
    }
    else if (format == "PDF") {
        model->exportPDF(filename, false);
    }
    else if (format == "SVG") {
        model->exportSVG(filename, false);
    }
    else if (format == "XML") {
        if (argumentsCount < 3) {
            scriptEngine->throwError(
                    QJSValue::GenericError,
                    "path missing in export to Impress");
            return setResult(r);
        }
        QString dpath = args[2].toString();
        model->exportXML(filename, dpath, false);
    }
    else {
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Unknown export format: %1").arg(format));
        return setResult(r);
    }
    return setResult(true);
}

BranchWrapper* VymModelWrapper::findBranchByAttribute(
        const QString &key,
        const QString &value)
{
    BranchItem *bi = model->findBranchByAttribute(key, value);
    if (bi)
        return bi->branchWrapper();
    else
        return nullptr;
}

BranchWrapper* VymModelWrapper::findBranchById(const QString &u)
{
    TreeItem *ti = model->findUuid(QUuid(u));
    if (ti && ti->hasTypeBranch())
        return ((BranchItem*)ti)->branchWrapper();
    else
        return nullptr;
}

BranchWrapper* VymModelWrapper::findBranchBySelection(const QString &s)
{
    TreeItem *ti = model->findBySelectString(s);
    if (ti && ti->hasTypeBranch())
        return ((BranchItem*)ti)->branchWrapper();
    else
        return nullptr;
}

ImageWrapper* VymModelWrapper::findImageById(const QString &u)
{
    TreeItem *ti = model->findUuid(QUuid(u));
    if (ti && ti->hasTypeImage())
        return ((ImageItem*)ti)->imageWrapper();
    else
        return nullptr;
}

ImageWrapper* VymModelWrapper::findImageBySelection(const QString &s)
{
    TreeItem *ti = model->findBySelectString(s);
    if (ti && ti->hasTypeImage())
        return ((ImageItem*)ti)->imageWrapper();
    else
        return nullptr;
}

/*
XLinkWrapper* VymModelWrapper::findXLinkById(const QString &u)
{
    TreeItem *ti = model->findUuid(QUuid(u));
    if (ti && ti->hasTypeXLink())
        return ((XLinkItem*)ti)->getXLink()->xlinkWrapper();
    else
        return nullptr;
}
*/
QString VymModelWrapper::getDestPath()
{
    QString r = model->getDestPath();
    return setResult(r);
}

QString VymModelWrapper::getFileDir() { return setResult(model->getFileDir()); }

QString VymModelWrapper::getFileName()
{
    return setResult(model->getFileName());
}

QString VymModelWrapper::getHeadingXML()
{
    QString r = model->getHeading().saveToDir();
    return setResult(r);
}

QString VymModelWrapper::getAuthor()
{
    return setResult(model->getAuthor());
}

QString VymModelWrapper::getComment()
{
    return setResult(model->getComment());
}

QString VymModelWrapper::getTitle() { return setResult(model->getTitle()); }

QString VymModelWrapper::getNotePlainText()
{
    return setResult(model->getNote().getTextASCII());
}

QString VymModelWrapper::getNoteXML()
{
    return setResult(model->getNote().saveToDir());
}

qreal VymModelWrapper::getScenePosX()
{
    Container *c = nullptr;
    BranchItem *selbi = model->getSelectedBranch();
    if (selbi)
        c = (Container*)(selbi->getBranchContainer());
    else {
        ImageItem *selii = model->getSelectedImage();
        if (selii)
            c = (Container*)(selii->getImageContainer());
    }

    if (c)
        return setResult(c->scenePos().x());

    scriptEngine->throwError(
            QJSValue::GenericError,
            "Could not get scenePos.x() from item");
    return 0;
}

qreal VymModelWrapper::getScenePosY()
{
    Container *c = nullptr;
    BranchItem *selbi = model->getSelectedBranch();
    if (selbi)
        c = (Container*)(selbi->getBranchContainer());
    else {
        ImageItem *selii = model->getSelectedImage();
        if (selii)
            c = (Container*)(selii->getImageContainer());
    }

    if (c)
        return setResult(c->scenePos().y());

    scriptEngine->throwError(
            QJSValue::GenericError,
            "Could not get scenePos.y() from item");
    return 0;
}

int VymModelWrapper::getRotationHeading()
{
    int r = -1;
    BranchItem *selbi = model->getSelectedBranch();
    if (selbi)
        r = selbi->getBranchContainer()->rotationHeading();
    else
        scriptEngine->throwError(QJSValue::RangeError, QString("No branch selected"));
    return setResult(r);
}

int VymModelWrapper::getRotationSubtree()
{
    int r = -1;
    BranchItem *selbi = model->getSelectedBranch();
    if (selbi)
        r = selbi->getBranchContainer()->rotationSubtree();
    else
        scriptEngine->throwError(QJSValue::RangeError, QString("No branch selected"));
    return setResult(r);
}

QString VymModelWrapper::getSelectionString()
{
    return setResult(model->getSelectString());
}

bool VymModelWrapper::loadBranchReplace(QString fileName, BranchWrapper *bw)
{
    if (QDir::isRelativePath(fileName))
        fileName = QDir::currentPath() + "/" + fileName;

    return setResult(model->addMapReplace(fileName, bw->branchItem()));
}

bool VymModelWrapper::loadDataInsert(QString fileName)
{
    if (QDir::isRelativePath(fileName))
        fileName = QDir::currentPath() + "/" + fileName;

    return setResult(model->addMapInsert(fileName));
}

void VymModelWrapper::newBranchIterator(const QString &itname, bool deepLevelsFirst)
{
    model->newBranchIterator(itname, deepLevelsFirst);
}

BranchWrapper* VymModelWrapper::nextBranch(const QString &itname)
{
    BranchItem *bi = model->nextBranchIterator(itname);
    if (bi)
        return bi->branchWrapper();
    else
        return nullptr;
}

void VymModelWrapper::moveSlideDown(int n)
{
    if (!model->moveSlideDown(n))
        scriptEngine->throwError(
                QJSValue::GenericError,
                "Could not move slide down");
}

void VymModelWrapper::moveSlideDown() { moveSlideDown(-1); }

void VymModelWrapper::moveSlideUp(int n)
{
    if (!model->moveSlideUp(n))
        scriptEngine->throwError(
                QJSValue::GenericError,
                "Could not move slide up");
}

void VymModelWrapper::moveSlideUp() { moveSlideUp(-1); }

void VymModelWrapper::note2URLs() { model->note2URLs(); }

void VymModelWrapper::paste() { model->paste(); }

void VymModelWrapper::redo() { model->redo(); }

void VymModelWrapper::remove() { model->deleteSelection(); }

void VymModelWrapper::removeBranch(BranchWrapper *bw)
{
    if (!bw) {
        scriptEngine->throwError(
                QJSValue::GenericError,
                "VymModelWrapper::removeBranch(b) b is invalid");
        return;
    }
    model->deleteSelection(bw->branchItem()->getID());
}

void VymModelWrapper::removeImage(ImageWrapper *iw)
{
    if (!iw) {
        scriptEngine->throwError(
                QJSValue::GenericError,
                "VymModelWrapper::removeImage(i) i is invalid");
        return;
    }
    model->deleteSelection(iw->imageItem()->getID());
}

void VymModelWrapper::removeKeepChildren(BranchWrapper *bw)
{
    model->deleteKeepChildren(bw->branchItem());
}

void VymModelWrapper::removeSlide(int n)
{
    if (n < 0 || n >= model->slideCount() - 1)
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Slide '%1' not available.").arg(n));
}

/*
void VymModelWrapper::removeXLink(XLinkWrapper *xlw)
{
    if (!xlw) {
        scriptEngine->throwError(
                QJSValue::GenericError,
                "VymModelWrapper::removeXLink(xl) xlink is invalid");
        return;
    }
    model->deleteXLink(xlw->xlink());
}
*/
QVariant VymModelWrapper::repeatLastCommand()
{
    return model->repeatLastCommand();
}

void VymModelWrapper::saveImage(const QString &filename)
{
    model->saveImage(nullptr, filename);
}

void VymModelWrapper::saveNote(const QString &filename)
{
    model->saveNote(filename);
}

void VymModelWrapper::saveSelection(const QString &filename)
{
    QString filename_org = model->getFilePath(); // Restore filename later
    if (!model->renameMap(filename)) {
        QString s = tr("Saving the selection in map failed:\nCouldn't rename map to %1").arg(filename);
        QMessageBox::critical(0,
            tr("Critical Error"), s);
        scriptEngine->throwError(QJSValue::GenericError, s);
        return;
    }
    model->saveMap(File::PartOfMap);
    model->renameMap(filename_org);
}

bool VymModelWrapper::select(const QString &s)
{
    bool r = model->select(s);
    if (!r)
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Couldn't select %1").arg(s));
    return setResult(r);
}

AttributeWrapper* VymModelWrapper::selectedAttribute()
{
    AttributeItem *ai = model->getSelectedAttribute();

    if (ai)
        return ai->attributeWrapper();
    else
        return nullptr;
}

BranchWrapper* VymModelWrapper::selectedBranch()
{
    BranchItem *selbi = model->getSelectedBranch();

    if (selbi)
        return selbi->branchWrapper();
    else
        return nullptr; // caught by QJSEngine
}

FooWrapper* VymModelWrapper::selectedFoo()
{
    return new FooWrapper();
}

/*
XLinkWrapper* VymModelWrapper::selectedXLink()
{
    XLinkItem *xli = model->getSelectedXLinkItem();

    if (xli)
        return xli->getXLink()->xlinkWrapper();
    else
        return nullptr;
}
*/

bool VymModelWrapper::selectUids(QJSValueList args)
{
    int argumentsCount = args.count();

    if (argumentsCount == 0) {
        scriptEngine->throwError(
                QJSValue::GenericError,
                "Not enough arguments");
        return setResult(false);
    }

    QStringList uids;
    foreach (auto arg, args)
        uids << arg.toString();

    bool r = model->selectUids(uids);
    if (!r)
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Couldn't select Uuids: %1").arg(uids.join(",")));
    return setResult(r);
}

bool VymModelWrapper::selectLatestAdded()
{
    bool r = model->selectLatestAdded();
    if (!r)
        scriptEngine->throwError(
                QJSValue::GenericError,
                "Couldn't select latest added item");
    return setResult(r);
}

bool VymModelWrapper::selectToggle(const QString &selectString)
{
    bool r = model->selectToggle(selectString);
    if (!r)
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Couldn't toggle item with select string \"%1\"").arg(selectString));
    return setResult(r);
}

void VymModelWrapper::setDefaultLinkColor(const QString &color)
{
    QColor col(color);
    if (col.isValid()) {
        model->setDefaultLinkColor(col);
    }
    else
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Could not set color to %1").arg(color));
}

void VymModelWrapper::setHeadingConfluencePageName()
{
    model->setHeadingConfluencePageName();
}

void VymModelWrapper::setHideExport(bool b) { model->setHideExport(b); }

void VymModelWrapper::setHideLinkUnselected(bool b)
{
    model->setHideLinkUnselected(b);
}

void VymModelWrapper::setAnimCurve(int n)
{
    if (n < 0 || n > QEasingCurve::OutInBounce)
        scriptEngine->throwError(
                QJSValue::RangeError,
                QString("Unknown animation curve type: ").arg(n));
    else {
        QEasingCurve c;
        c.setType((QEasingCurve::Type)n);
        model->setMapAnimCurve(c);
    }
}

void VymModelWrapper::setAnimDuration(int n)
{
    model->setMapAnimDuration(n);
}

void VymModelWrapper::setAuthor(const QString &s) { model->setAuthor(s); }

void VymModelWrapper::setBackgroundColor(const QString &color)
{
    QColor col(color);
    if (col.isValid()) {
        model->setBackgroundColor(col);
    }
    else
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Could not set color to %1").arg(color));
}

void VymModelWrapper::setComment(const QString &s) { model->setComment(s); }

void VymModelWrapper::setLinkStyle(const QString &style)
{
    if (!model->setLinkStyle(style))
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Could not set linkstyle to %1").arg(style));
}

void VymModelWrapper::setRotation(float a) { model->setMapRotation(a); }

void VymModelWrapper::setTitle(const QString &s) { model->setTitle(s); }

void VymModelWrapper::setZoom(float z) { model->setMapZoomFactor(z); }

void VymModelWrapper::setNotePlainText(const QString &s)
{
    VymNote vn;
    vn.setPlainText(s);
    model->setNote(vn);
}

void VymModelWrapper::setRotationHeading(const int &i)
{
    model->setRotationHeading(i);
}

void VymModelWrapper::setRotationSubtree(const int &i)
{
    model->setRotationSubtree(i);
}

void VymModelWrapper::setRotationsAutoDesign(const bool b)
{
    model->setRotationsAutoDesign(b);
}

void VymModelWrapper::setScale(qreal f) { model->setScale(f, false); }

void VymModelWrapper::setScaleSubtree(qreal f) { model->setScaleSubtree(f); }

void VymModelWrapper::setScalingAutoDesign(const bool b)
{
    model->setScaleAutoDesign(b);
}

void VymModelWrapper::setSelectionBrushColor(const QString &color)
{
    QColor col(color);
    if (!col.isValid())
        //logErrorOld(context(), QScriptContext::SyntaxError,
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Could not set color to %1").arg(color));
    else
        model->setSelectionBrushColor(col);
}

void VymModelWrapper::setSelectionPenColor(const QString &color)
{
    QColor col(color);
    if (!col.isValid())
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Could not set color to %1").arg(color));
    else
        model->setSelectionPenColor(col);
}

void VymModelWrapper::setSelectionPenWidth(const qreal &w)
{
    model->setSelectionPenWidth(w);
}

void VymModelWrapper::sleep(int n)
{
    // FIXME-5 sleep is not avail on windows VCEE, workaround could be using
    // this->thread()->wait(x ms)
    sleep(n);
}

int VymModelWrapper::slideCount()
{
    return setResult(model->slideCount());
}

void VymModelWrapper::toggleTarget() { model->toggleTarget(); }

void VymModelWrapper::undo() { model->undo(); }

void VymModelWrapper::unselectAll() { model->unselectAll(); }

