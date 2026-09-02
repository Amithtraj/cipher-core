/* Copyright (c) 2025 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.toolbar.adaptive;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.view.View;

import org.chromium.base.supplier.MonotonicObservableSupplier;
import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.ActivityTabProvider;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.toolbar.optional_button.BaseButtonDataProvider;
import org.chromium.chrome.browser.toolbar.optional_button.ButtonData.ButtonSpec;
import org.chromium.ui.modaldialog.ModalDialogManager;

/** Handles displaying Brave Wallet button on toolbar. */
@NullMarked
public class BraveWalletButtonController extends BaseButtonDataProvider {
    public BraveWalletButtonController(
            Context context,
            Drawable buttonDrawable,
            ActivityTabProvider tabProvider,
            MonotonicObservableSupplier<Profile> profileSupplier,
            ModalDialogManager modalDialogManager) {
        super(
                tabProvider,
                modalDialogManager,
                new ButtonSpec.Builder(
                                buttonDrawable,
                                context.getString(R.string.menu_brave_wallet),
                                /* supportsTinting= */ true)
                        .setButtonVariant(AdaptiveToolbarButtonVariant.WALLET)
                        .setHoverTooltipTextId(R.string.menu_brave_wallet)
                        .build());
    }

    @Override
    public void onClick(View view) {
        // Wallet is not compiled into this build (ENABLE_BRAVE_WALLET=false); shouldShowButton()
        // always returns false so this is unreachable, but is kept as a harmless no-op.
    }

    @Override
    protected boolean shouldShowButton(@Nullable Tab tab) {
        // Wallet is not compiled into this build (ENABLE_BRAVE_WALLET=false); never show the
        // adaptive toolbar Wallet button.
        return false;
    }
}
