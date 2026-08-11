/* Copyright (c) 2026 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_vpn/browser/v2/brave_vpn_service_impl.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "base/check.h"
#include "base/check_deref.h"
#include "base/functional/bind.h"
#include "base/notimplemented.h"
#include "base/sequence_checker.h"
#include "base/types/to_address.h"
#include "brave/components/brave_vpn/browser/v2/api/brave_vpn_api_client.h"
#include "brave/components/brave_vpn/browser/v2/purchased_state_manager.h"
#include "brave/components/brave_vpn/browser/v2/skus_service_client.h"
#include "brave/components/brave_vpn/common/brave_vpn_utils.h"
#include "build/build_config.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace brave_vpn::v2 {

BraveVpnServiceImpl::BraveVpnServiceImpl(
    PrefService* local_prefs,
    PrefService* profile_prefs,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    GetSkusServiceCallback skus_service_getter)
    : profile_prefs_(CHECK_DEREF(profile_prefs)),
      api_client_(
          std::make_unique<BraveVpnApiClient>(std::move(url_loader_factory))),
      skus_client_(
          std::make_unique<SkusServiceClient>(std::move(skus_service_getter))),
      connection_state_(mojom::ConnectionState::DISCONNECTED) {
  DCHECK(IsBraveVPNFeatureEnabled());
#if !BUILDFLAG(IS_ANDROID)
  agent_client_ = std::make_unique<AgentClient>();
  agent_client_->AddObserver(this);
#endif  // !BUILDFLAG(IS_ANDROID)
  purchased_state_manager_ = std::make_unique<PurchasedStateManager>(
      local_prefs, api_client_.get(), skus_client_.get(),
      base::BindRepeating(&BraveVpnServiceImpl::OnPurchasedStateChanged,
                          base::Unretained(this)));
}

BraveVpnServiceImpl::~BraveVpnServiceImpl() = default;

bool BraveVpnServiceImpl::IsBraveVPNEnabled() const {
  return ::brave_vpn::IsBraveVPNEnabled(base::to_address(profile_prefs_));
}

bool BraveVpnServiceImpl::IsPurchased() const {
  if (!purchased_state_manager_) {
    return false;
  }
  return purchased_state_manager_->IsPurchased();
}

void BraveVpnServiceImpl::ReloadPurchasedState() {
  if (purchased_state_manager_) {
    purchased_state_manager_->Reload();
  }
}

std::string BraveVpnServiceImpl::GetCurrentEnvironment() const {
  if (!purchased_state_manager_) {
    return {};
  }
  return purchased_state_manager_->GetCurrentEnvironment();
}

void BraveVpnServiceImpl::GetPurchasedState(
    GetPurchasedStateCallback callback) {
  if (purchased_state_manager_) {
    std::move(callback).Run(purchased_state_manager_->GetInfo().Clone());
    return;
  }
  std::move(callback).Run(mojom::PurchasedInfo::New(
      mojom::PurchasedState::NOT_PURCHASED, std::nullopt));
}

void BraveVpnServiceImpl::LoadPurchasedState(const std::string& domain) {
  if (purchased_state_manager_) {
    purchased_state_manager_->Load(domain);
  }
}

void BraveVpnServiceImpl::GetAllRegions(GetAllRegionsCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run({});
}

void BraveVpnServiceImpl::Shutdown() {
#if !BUILDFLAG(IS_ANDROID)
  if (agent_client_) {
    agent_client_->RemoveObserver(this);
    agent_client_.reset();
  }
#endif  // !BUILDFLAG(IS_ANDROID)
  purchased_state_manager_.reset();
  api_client_.reset();
  skus_client_->Reset();
  BraveVpnService::Shutdown();
}

void BraveVpnServiceImpl::OnPurchasedStateChanged(
    mojom::PurchasedState state,
    std::optional<std::string> description) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

#if !BUILDFLAG(IS_ANDROID)
  UpdateAgentConnection(state);
#endif  // !BUILDFLAG(IS_ANDROID)

  NotifyPurchasedStateChanged(state, description);

  // TODO: If purchased state changed to PURCHASED on desktop, we can attempt to
  // install VPN helper, etc. We also need to make sure the agent, once
  // connected, fetches the region data - BEFORE we actually send a notification
  // to the UI.
}

}  // namespace brave_vpn::v2
