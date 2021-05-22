/****************************************************************************
**
** Copyright (C) 2021 Ivan Komissarov (abbapoh@gmail.com)
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PKGCONFIG_H
#define PKGCONFIG_H

#include "pcpackage.h"

namespace qbs {

class PkgConfig
{
public:
    struct Options {
        using VariablesMap = PcPackage::VariablesMap;

        std::vector<std::string> searchPaths;        // PKG_CONFIG_PATH, PKG_CONFIG_LIBDIR
        std::string sysroot;                         // PKG_CONFIG_SYSROOT_DIR
        std::string topBuildDir;                     // PKG_CONFIG_TOP_BUILD_DIR
        bool allowSystemLibraryPaths{false};         // PKG_CONFIG_ALLOW_SYSTEM_LIBS
        std::vector<std::string> systemLibraryPaths; // PKG_CONFIG_SYSTEM_LIBRARY_PATH
        bool disableUninstalled{true};               // PKG_CONFIG_DISABLE_UNINSTALLED
        VariablesMap globalVariables;
        VariablesMap systemVariables;
    };

    using Packages = std::vector<PcPackage>;
    using BrokenPackages = std::vector<PcBrokenPackage>;

    explicit PkgConfig();
    explicit PkgConfig(Options options);

    const Options &options() const { return m_options; }
    const Packages &packages() const { return m_packages; }
    const BrokenPackages &brokenPackages() const { return m_brokenPackages; }
    const PcPackage &getPackage(std::string_view baseFileName) const;

    std::string_view packageGetVariable(const PcPackage &pkg, std::string_view var) const;

private:
    std::pair<Packages, BrokenPackages> findPackages() const;

private:
    Options m_options;

    Packages m_packages;
    BrokenPackages m_brokenPackages;
};

} // namespace qbs

#endif // PKGCONFIG_H
