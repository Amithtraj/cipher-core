/* Copyright (c) 2025 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.components.minidump_uploader;

import android.net.Uri;

import androidx.test.filters.SmallTest;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

import org.chromium.base.test.BaseRobolectricTestRunner;
import org.chromium.base.version_info.VersionInfo;
import org.chromium.components.minidump_uploader.util.BraveHttpURLConnectionFactoryImpl;
import org.chromium.components.minidump_uploader.util.HttpURLConnectionFactory;

import java.net.HttpURLConnection;

/** Unittests for {@link BraveHttpURLConnectionFactoryImpl}. */
@RunWith(BaseRobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class BraveHttpURLConnectionFactoryImplTest {
    @Test
    @SmallTest
    public void testUploadUrlHasProductVersionGuid() {
        // Cipher disables the crash-report upload URL entirely for non-official builds (see
        // BraveHttpURLConnectionFactoryImpl), so force an official build here to exercise the
        // URL-construction logic this test actually verifies.
        VersionInfo.setOverridesForTesting(/* official= */ true, /* stable= */ null, /* local= */ null);
        HttpURLConnectionFactory httpURLConnectionFactory = new BraveHttpURLConnectionFactoryImpl();
        HttpURLConnection connection = httpURLConnectionFactory.createHttpURLConnection("");
        Assert.assertNotNull(connection);
        Uri uri = Uri.parse(connection.getURL().toString());
        Assert.assertEquals("Brave_Android", uri.getQueryParameter("product"));
        Assert.assertEquals(VersionInfo.getProductVersion(), uri.getQueryParameter("version"));
        Assert.assertEquals("00000000-0000-0000-0000-000000000000", uri.getQueryParameter("guid"));
    }

    @Test
    @SmallTest
    public void testUploadUrlDisabledForNonOfficialBuild() {
        // Non-official builds (which is what every Cipher build is) must never construct a
        // real crash-report upload URL: BraveHttpURLConnectionFactoryImpl.createHttpURLConnection
        // returns null unless VersionInfo.isOfficialBuild() is true.
        VersionInfo.setOverridesForTesting(/* official= */ false, /* stable= */ null, /* local= */ null);
        HttpURLConnectionFactory httpURLConnectionFactory = new BraveHttpURLConnectionFactoryImpl();
        Assert.assertNull(httpURLConnectionFactory.createHttpURLConnection(""));
    }
}
