/* Copyright (c) 2025 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.components.minidump_uploader.util;

import org.chromium.base.version_info.VersionInfo;
import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;

import java.net.HttpURLConnection;

@NullMarked
public class BraveHttpURLConnectionFactoryImpl extends HttpURLConnectionFactoryImpl {
    // Guid is intentionally zeroed so we couldn't identify our users
    static final String CRASH_URL_STRING_TEMPLATE =
            "https://cr.brave.com/?product=Brave_Android&version=%s&guid=00000000-0000-0000-0000-000000000000";

    @Override
    public @Nullable HttpURLConnection createHttpURLConnection(String url) {
        // Disabled for Cipher: mirror the C++ path in
        // chromium_src/components/crash/core/app/crash_reporter_client.cc, which returns an
        // empty upload URL (no crash server) for non-official builds. This build is never
        // officially signed/branded, so this guard permanently disables the upload target;
        // VersionInfo.isOfficialBuild() is already the Java-visible equivalent of the C++
        // OFFICIAL_BUILD macro used there.
        if (!VersionInfo.isOfficialBuild()) {
            return null;
        }
        String version = VersionInfo.getProductVersion();
        String braveUploadUrl = String.format(CRASH_URL_STRING_TEMPLATE, version);
        return super.createHttpURLConnection(braveUploadUrl);
    }
}
