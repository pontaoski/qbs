/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qbs.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "cxxmodulesscanner.h"

#include "artifact.h"
#include "depscanner.h"
#include "productbuilddata.h"
#include "projectbuilddata.h"
#include "rawscanresults.h"
#include <language/scriptengine.h>
#include <logging/categories.h>
#include <logging/translator.h>
#include <plugins/scanner/scanner.h>
#include <tools/fileinfo.h>
#include <tools/scannerpluginmanager.h>
#include <tools/scripttools.h>

#include <QtCore/qdebug.h>

#include <QtScript/qscriptcontext.h>
#include <QtScript/qscriptengine.h>

namespace qbs
{

namespace Internal
{

class Cxx20ModulesScanner : public DependencyScanner
{
public:
    Cxx20ModulesScanner(const DependencyScanner &actualScanner)
        : m_id(QStringLiteral("cxx20modules") + actualScanner.id()) {}

private:
    QStringList collectSearchPaths(Artifact *) override { return {}; }
    QStringList collectDependencies(Artifact *, FileResourceBase *, const char *) override
    {
        return {};
    }
    bool recursive() const override { return false; }
    const void *key() const override { return nullptr; }
    QString createId() const override { return m_id; }
    bool areModulePropertiesCompatible(const PropertyMapConstPtr &,
                                       const PropertyMapConstPtr &) const override
    {
        return true;
    }
    bool cacheIsPerFile() const override { return false; }

    const QString m_id;
};

static QString cxx20ModulesScannerJsName() { return QStringLiteral("Cxx20ModulesScanner"); }

CxxModulesScanner::CxxModulesScanner(const ResolvedProductPtr &product, QScriptValue targetScriptValue)
    : m_product(product)
    , m_targetScriptValue(targetScriptValue)
{
    const auto engine = static_cast<ScriptEngine *>(targetScriptValue.engine());
    QScriptValue scannerObj = engine->newObject();
    targetScriptValue.setProperty(cxx20ModulesScannerJsName(), scannerObj);
    QScriptValue applyFunction = engine->newFunction(&js_apply, this);
    scannerObj.setProperty(QStringLiteral("scan"), applyFunction);
}

CxxModulesScanner::~CxxModulesScanner()
{
    m_targetScriptValue.setProperty(cxx20ModulesScannerJsName(), QScriptValue());
}

static RawScanResult runScanner(ScannerPlugin *scanner, const Artifact *artifact)
{
    const QString &filepath = artifact->filePath();
    Cxx20ModulesScanner depScanner((PluginDependencyScanner(scanner)));
    RawScanResults &rawScanResults
            = artifact->product->topLevelProject()->buildData->rawScanResults;
    RawScanResults::ScanData &scanData = rawScanResults.findScanData(artifact, &depScanner,
                                                                     artifact->properties);
    if (scanData.lastScanTime < artifact->timestamp()) {
        FileTags tags = artifact->fileTags();
        if (tags.contains("cpp.combine")) {
            tags.remove("cpp.combine");
            tags.insert("cpp");
        }

        const QByteArray tagsForScanner = tags.toStringList().join(QLatin1Char(',')).toLatin1();
        void *opaq = scanner->open(filepath.utf16(), tagsForScanner.constData(),
                                   ScanForDependenciesFlag | ScanForFileTagsFlag);

        if (!opaq || !scanner->moduleInformation)
            return scanData.rawScanResult;

        auto moduleInformation = scanner->moduleInformation(opaq);

        scanData.rawScanResult.belongsToModule = moduleInformation.belongsToModule;
        scanData.rawScanResult.exportsModule = moduleInformation.exportsModule;
        scanData.rawScanResult.importsModules = moduleInformation.importsModules;
        scanData.rawScanResult.importsSubmodules = moduleInformation.importsSubmodules;

        scanner->close(opaq);
        scanData.lastScanTime = FileTime::currentTime();
    }
    return scanData.rawScanResult;
}

static QScriptValue scannerCountError(QScriptEngine *engine, size_t scannerCount,
        const QString &fileTag)
{
    return engine->currentContext()->throwError(
                Tr::tr("There are %1 scanners for the file tag %2. "
                       "Expected is exactly one.").arg(scannerCount).arg(fileTag));
}

QScriptValue CxxModulesScanner::js_apply(QScriptContext *ctx, QScriptEngine *engine,
                                         CxxModulesScanner *that)
{
    QScriptValue input = ctx->argument(0);
    return that->apply(engine, attachedPointer<Artifact>(input));
}

QScriptValue CxxModulesScanner::apply(QScriptEngine* engine, const Artifact* artifact)
{
    if (!m_cppScanner) {
        auto scanners = ScannerPluginManager::scannersForFileTag(cxxTag);
        if (scanners.size() != 1)
            return scannerCountError(engine, scanners.size(), QStringLiteral("cpp"));
        m_cppScanner = scanners.front();
    }

    RawScanResult scanResult = runScanner(m_cppScanner, artifact);
    QScriptValue obj = engine->newObject();

    obj.setProperty(QStringLiteral("exportsModule"), scanResult.exportsModule);
    obj.setProperty(QStringLiteral("belongsToModule"), scanResult.belongsToModule);
    obj.setProperty(QStringLiteral("importsSubmodules"), qScriptValueFromSequence(engine, scanResult.importsSubmodules));
    obj.setProperty(QStringLiteral("importsModules"), qScriptValueFromSequence(engine, scanResult.importsModules));

    static_cast<ScriptEngine *>(engine)->setUsesIo();

    return obj;
}

}; // namespace Internal

}; // namespace qbs
