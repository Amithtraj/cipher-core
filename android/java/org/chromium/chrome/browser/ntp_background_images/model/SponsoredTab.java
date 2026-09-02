/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.ntp_background_images.model;

import org.chromium.chrome.browser.ntp_background_images.NTPBackgroundImagesBridge;

// SecureOut: this class is now only used as a marker of whether the NTP background-image
// feature has been initialized for a tab (see the `mSponsoredTab != null` checks in
// BraveNewTabPageLayout/BraveNtpAdapter) -- BraveNewTabPageLayout.getAndShowNTPImage() no
// longer calls into NTPBackgroundImagesBridge to fetch a wallpaper at all, so the
// getNTPImage()/getNTPImageCallback() methods that used to do that (and the bridge reference
// they needed) were removed as dead code. The constructor signature is left unchanged since
// it's still the marker BraveNewTabPageLayout.initilizeSponsoredTab() constructs.
public class SponsoredTab {
    public SponsoredTab(
            NTPBackgroundImagesBridge mNTPBackgroundImagesBridge, boolean allowSponsoredImage) {}
}
