// Copyright 2026 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

import BraveCore
import BraveShared
import DesignSystem
import Strings
import SwiftUI

class ShieldsPanelViewModel: ObservableObject {
  
}

public struct ShieldsPanelView: View {
  /// Called with the height required to display the panel's contents without scrolling
  public var onContentHeightChanged: ((CGFloat) -> Void)?

  private let url: URL
  private let displayHost: String
  private let numberOfTrackersBlocked: Int
  private let isAdvancedControlsEnabled: Bool
  private let isShredEnabled: Bool
  private let autoShredLevel: SiteShredLevel
  private var action: (ShieldsPanelAction) -> Void

  public init(
    url: URL,
    numberOfTrackersBlocked: Int,
    isAdvancedControlsEnabled: Bool,
    isShredEnabled: Bool,
    autoShredLevel: SiteShredLevel,
    action: @escaping (ShieldsPanelAction) -> Void
  ) {
    self.url = url
    self.numberOfTrackersBlocked = numberOfTrackersBlocked
    self.isAdvancedControlsEnabled = isAdvancedControlsEnabled
    self.isShredEnabled = isShredEnabled
    self.autoShredLevel = autoShredLevel
    self.action = action
    self.displayHost =
      "\u{200E}\(URLFormatter.formatURLOrigin(forDisplayOmitSchemePathAndTrivialSubdomains: url.strippingBlobURLAuth.absoluteString))"
  }

  @State private var isShieldsEnabled: Bool = true
  @AppStorage("advancedShieldsExpanded") private var isAdvancedControlsExpanded: Bool = false

  @State private var blockAdsAndTrackingLevel: ShieldLevel = .standard
  @State private var isBlockScriptsEnabled: Bool = false
  @State private var isBlockFingerprintingEnabled: Bool = true

  public var body: some View {
    Form {
      Toggle(isOn: $isShieldsEnabled) {
        VStack(alignment: .leading) {
          URLElidedText(text: displayHost)
            .font(.title2)
            .fontWeight(.semibold)
            .foregroundStyle(isShieldsEnabled ? .primary : .secondary)
          Group {
            Text(
              LocalizedStringKey(
                isShieldsEnabled
                  ? Strings.Shields.shieldsUpForSite : Strings.Shields.shieldsDownForSite
              )
            )
          }
          .font(.footnote)
          .foregroundStyle(.secondary)
        }
      }
      .tint(Color(braveSystemName: .primary40))
      .listRowBackground(Color.clear)

      if isShieldsEnabled {
        shieldsUpView
      } else {
        shieldsDownView
      }
    }
    .listSectionSpacing(.compact)
    // `isAdvancedControlsExpanded` is backed by UserDefaults, so its change lands outside of any
    // `withAnimation` transaction. Animate off the value instead.
    .animation(.default, value: isAdvancedControlsExpanded)
    // A `Form` has no usable ideal size (it always fills its container), so report the height its
    // contents actually need by observing the underlying scroll view.
    .onScrollGeometryChange(for: CGFloat.self) { geometry in
      geometry.contentSize.height + geometry.contentInsets.top + geometry.contentInsets.bottom
    } action: { _, height in
      onContentHeightChanged?(height)
    }
  }

  @ViewBuilder private var shieldsUpView: some View {
    Section {
      HStack {
        // TODO: Circles for sites blocked?
        Circle()
          .frame(width: 24)
        Text("\(42)")
          .font(.title2)
          .fontWeight(.semibold)
        Text(Strings.Shields.trackersAdsAndMoreBlocked)
          .font(.footnote)
      }
    }

    Section {
      if isAdvancedControlsEnabled {
        DisclosureGroup(isExpanded: $isAdvancedControlsExpanded) {
          advancedShieldsSection
        } label: {
          Text(Strings.Shields.advancedControls)
            .foregroundStyle(Color(braveSystemName: .textPrimary))
            .frame(maxWidth: .infinity, alignment: .leading)
        }
        .disclosureGroupStyle(ShieldsPanelDisclosureStyle())
      }
    } footer: {
      Text(
        String.localizedStringWithFormat(
          Strings.Shields.siteSeemsBroken,
          URL.Brave.privacyFeatures.absoluteString
        )
      )
      .foregroundStyle(Color.secondary)
      .font(.caption)
      .listRowBackground(Color.clear)
    }
  }

  @ViewBuilder private var advancedShieldsSection: some View {
    Picker(selection: $blockAdsAndTrackingLevel) {
      ForEach(ShieldLevel.allCases) { level in
        Text(level.localizedTitle)
          .tag(level)
      }
    } label: {
      Text(Strings.Shields.trackersAndAdsBlocking)
    }

    Toggle(Strings.Shields.blockScripts, isOn: $isBlockScriptsEnabled)
      .tint(Color(braveSystemName: .primary40))

    Toggle(Strings.Shields.fingerprintingProtection, isOn: $isBlockFingerprintingEnabled)
      .tint(Color(braveSystemName: .primary40))

    if isShredEnabled {
      NavigationLink {
        // TODO: Support `ShredSiteSettingsView`
        Color.red
      } label: {
        HStack {
          Text(Strings.Shields.shredSiteData)
            .multilineTextAlignment(.leading)
          Spacer()
          Text(autoShredLevel.localizedTitle)
            .multilineTextAlignment(.trailing)
            .foregroundStyle(.secondary)
        }
      }
    }

    Button {

    } label: {
      Label(Strings.Shields.shieldsGlobalSettingsButtonTitle, braveSystemImage: "leo.launch")
        .labelStyle(RightIconLabelStyle())
    }
    .buttonStyle(.plain)
  }

  @ViewBuilder private var shieldsDownView: some View {
    Section {
      HStack {
        Text(Strings.Shields.siteNotWorkingCorrectly)
        Button {

        } label: {
          Text(Strings.Shields.reportBrokenSiteButtonTitle)
        }
        .buttonStyle(.filled)
      }
    }
  }
}

#Preview(traits: .sizeThatFitsLayout) {
  NavigationStack {
    ShieldsPanelView(
      url: URL(string: "brave.com")!,
      numberOfTrackersBlocked: 42,
      isAdvancedControlsEnabled: true,
      isShredEnabled: true,
      autoShredLevel: .never,
      action: { _ in }
    )
    .toolbarVisibility(.hidden, for: .navigationBar)
  }
}

struct ShieldsPanelDisclosureStyle: DisclosureGroupStyle {
  func makeBody(configuration: Configuration) -> some View {
    Button {
      configuration.isExpanded.toggle()
    } label: {
      HStack {
        configuration.label
        Image(systemName: "chevron.down")
          .font(.body)
          .rotationEffect(.degrees(configuration.isExpanded ? -180 : 0))
      }
      .contentShape(Rectangle())
    }
    .buttonStyle(.plain)

    if configuration.isExpanded {
      configuration.content
        .transition(
          .asymmetric(
            insertion: .move(edge: .top).combined(with: .opacity),
            removal: .identity
          )
        )
    }
  }
}

private struct RightIconLabelStyle: LabelStyle {
  func makeBody(configuration: Configuration) -> some View {
    HStack {
      configuration.title
      Spacer()
      configuration.icon
    }
  }
}

public class ShieldsPanelViewController: UIHostingController<ShieldsPanelView> {
  public init(
    url: URL,
    isShredEnabled: Bool = true,
    isAdvancedControlsEnabled: Bool = true,
    action: @escaping (ShieldsPanelAction) -> Void
  ) {
    super.init(
      rootView: ShieldsPanelView(
        url: url,
        numberOfTrackersBlocked: 42,
        isAdvancedControlsEnabled: isAdvancedControlsEnabled,
        isShredEnabled: isShredEnabled,
        autoShredLevel: .never,
        action: action
      )
    )
    rootView.onContentHeightChanged = { [weak self] height in
      self?.updateContentHeight(height)
    }
  }

  @MainActor required dynamic init?(coder aDecoder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }

  /// The height the panel's contents need, as measured by SwiftUI. Zero until the first measurement
  /// lands.
  private var contentHeight: CGFloat = 0

  private var sheetController: UISheetPresentationController? {
    sheetPresentationController
      ?? popoverPresentationController?.adaptiveSheetPresentationController
  }

  private func updateContentHeight(_ height: CGFloat) {
    guard height > 0, height != contentHeight else { return }
    let isFirstMeasurement = contentHeight == 0
    contentHeight = height
    preferredContentSize = CGSize(width: 375, height: height)
    guard let sheetController else { return }
    if isFirstMeasurement {
      sheetController.invalidateDetents()
    } else {
      sheetController.animateChanges {
        sheetController.invalidateDetents()
      }
    }
  }

  public override func viewIsAppearing(_ animated: Bool) {
    super.viewIsAppearing(animated)

    if let sheetController {
      sheetController.detents = [
        .custom(identifier: .fitsContent) { [weak self] context in
          guard let height = self?.contentHeight, height > 0 else {
            return context.maximumDetentValue
          }
          return min(context.maximumDetentValue, height)
        },
        .large(),
      ]
      sheetController.prefersGrabberVisible = true
      sheetController.prefersEdgeAttachedInCompactHeight = true
    }
  }
}

extension UISheetPresentationController.Detent.Identifier {
  fileprivate static let fitsContent: Self = .init("fitsContent")
}
