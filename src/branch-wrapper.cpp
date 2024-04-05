#include "branch-wrapper.h"

#include "attributeitem.h"
#include "branchitem.h"

#include "vymmodel.h"

#include <QJSEngine>
extern QJSEngine *scriptEngine;

BranchWrapper::BranchWrapper(BranchItem *bi)
{
    //qDebug() << "Constr BranchWrapper (BI)";
    branchItem = bi;
}

BranchWrapper::~BranchWrapper()
{
    //qDebug() << "Destr BranchWrapper";
}

void BranchWrapper::addBranch()
{
    if (branchItem) {
        if (!branchItem->getModel()->addNewBranch(branchItem, -2))
        scriptEngine->throwError(QJSValue::GenericError,"Couldn't add branch to map");
        return;
    } else {
        scriptEngine->throwError(QJSValue::RangeError, QString("No branch selected"));
        return;
    }
}

void BranchWrapper::addBranchAt(int pos)
{
    if (!branchItem->getModel()->addNewBranch(branchItem, pos))
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Could not add  branch at position %1").arg(pos));
}

void BranchWrapper::addBranchBefore()
{
    if (!branchItem->getModel()->addNewBranchBefore(branchItem))
        scriptEngine->throwError(
                QJSValue::GenericError,
                "Couldn't add branch before selection to map");
}

int BranchWrapper::attributeAsInt(const QString &key)
{
    QVariant v;
    AttributeItem *ai = branchItem->getModel()->getAttributeByKey(key);
    if (ai) {
        v = ai->value();
    } else {
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("No attribute found with key '%1'").arg(key));
        return setResult(-1);
    }

    bool ok;
    int i = v.toInt(&ok);
    if (ok)
        return setResult(v.toInt());
    else {
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("Could not convert attribute  with key '%1' to int.").arg(key));
        return setResult(-1);
    }
}

QString BranchWrapper::attributeAsString(const QString &key)
{
    QVariant v;
    AttributeItem *ai = branchItem->getModel()->getAttributeByKey(key);
    if (ai) {
        v = ai->value();
    } else {
        scriptEngine->throwError(
                QJSValue::GenericError,
                QString("No attribute found with key '%1'").arg(key));
        return setResult(QString());
    }
    //
    // Returned string will be empty for unsupported variant types
    return setResult(v.toString());
}


QString BranchWrapper::headingText()
{
    return setResult(branchItem->headingPlain());
}

bool BranchWrapper::isScrolled()
{
    return setResult(branchItem->isScrolled());
}

bool BranchWrapper::relinkTo(BranchWrapper *bw)
{
    branchItem->getModel()->relinkBranch(branchItem, bw->branchItem);
    return false;
}

void BranchWrapper::scroll()
{
    branchItem->getModel()->scrollBranch(branchItem);
}

void BranchWrapper::select()
{
    branchItem->getModel()->select(branchItem);
}

void BranchWrapper::setFlagByName(const QString &s)
{
    branchItem->getModel()->setFlagByName(s, branchItem);
}

void BranchWrapper::toggleFlagByName(const QString &s)
{
    branchItem->getModel()->toggleFlagByName(s, branchItem);
}

void BranchWrapper::toggleScroll()
{
    branchItem->getModel()->toggleScroll(branchItem);
}

void BranchWrapper::unscroll()
{
    branchItem->getModel()->unscrollBranch(branchItem);
}

void BranchWrapper::unsetFlagByName(const QString &s)
{
    branchItem->getModel()->unsetFlagByName(s, branchItem);
}
