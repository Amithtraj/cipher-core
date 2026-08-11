/* Copyright (c) 2026 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/notimplemented.h"
#include "base/sequence_checker.h"
#include "base/types/expected.h"
#include "brave/components/brave_vpn/browser/v2/api/brave_vpn_api_client.h"
#include "brave/components/brave_vpn/browser/v2/brave_vpn_service_impl.h"
#include "brave/components/brave_vpn/browser/v2/purchased_state_manager.h"
#include "brave/components/brave_vpn/common/brave_vpn_constants.h"
#include "brave/components/brave_vpn/common/brave_vpn_utils.h"
#include "third_party/icu/source/i18n/unicode/timezone.h"

namespace brave_vpn::v2 {
namespace {
std::string GetTimeZoneName() {
  std::unique_ptr<icu::TimeZone> zone(icu::TimeZone::createDefault());
  icu::UnicodeString id;
  zone->getID(id);
  std::string current_time_zone;
  id.toUTF8String<std::string>(current_time_zone);
  return current_time_zone;
}

// Bridges the endpoint client's typed result to the mojom
// (bool success, string response) contract used by CreateSupportTicket.
void RunCreateSupportTicketCallback(
    BraveVpnServiceImpl::CreateSupportTicketCallback callback,
    base::expected<std::string, std::string> result) {
  const bool success = result.has_value();
  std::move(callback).Run(success,
                          success ? std::string() : std::move(result).error());
}

std::string_view BrowserAuthResultToString(
    std::optional<mojom::BrowserAuthResult> result) {
  if (!result.has_value()) {
    return "unknown";
  }
  switch (result.value()) {
    case mojom::BrowserAuthResult::kAccepted:
      return "accepted";
    case mojom::BrowserAuthResult::kRejected:
      return "rejected";
    case mojom::BrowserAuthResult::kVersionMismatch:
      return "version mismatch";
    case mojom::BrowserAuthResult::kHostAlreadyRequested:
      return "host already requested";
  }
}
}  // namespace

bool BraveVpnServiceImpl::IsConnected() const {
  NOTIMPLEMENTED();
  return connection_state_ == mojom::ConnectionState::CONNECTED &&
         IsPurchased();
}

void BraveVpnServiceImpl::ToggleConnection() {
  NOTIMPLEMENTED();
}

mojom::ConnectionState BraveVpnServiceImpl::GetConnectionState() const {
  NOTIMPLEMENTED();
  return connection_state_;
}

std::string BraveVpnServiceImpl::GetLastConnectionError() const {
  NOTIMPLEMENTED();
  return std::string();
}

void BraveVpnServiceImpl::RecordWidgetUsageMetrics(bool new_usage) {
  NOTIMPLEMENTED();
}

void BraveVpnServiceImpl::GetConnectionState(
    GetConnectionStateCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(connection_state_);
}

void BraveVpnServiceImpl::Connect() {
  NOTIMPLEMENTED();
}

void BraveVpnServiceImpl::Disconnect() {
  NOTIMPLEMENTED();
}

void BraveVpnServiceImpl::GetSelectedRegion(
    GetSelectedRegionCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(mojom::Region::New());
}

void BraveVpnServiceImpl::SetSelectedRegion(mojom::RegionPtr region) {
  NOTIMPLEMENTED();
}

void BraveVpnServiceImpl::ClearSelectedRegion() {
  NOTIMPLEMENTED();
}

void BraveVpnServiceImpl::GetProductUrls(GetProductUrlsCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(mojom::ProductUrls::New(
      kFeedbackUrl, kAboutUrl, GetManageUrl(GetCurrentEnvironment())));
}

void BraveVpnServiceImpl::CreateSupportTicket(
    const std::string& email,
    const std::string& subject,
    const std::string& body,
    CreateSupportTicketCallback callback) {
  if (!api_client_) {
    std::move(callback).Run(false, {});
    return;
  }
  std::optional<std::string> subscriber_credential =
      purchased_state_manager_
          ? purchased_state_manager_->GetSubscriberCredential()
          : std::nullopt;

  api_client_->CreateSupportTicket(
      base::BindOnce(&RunCreateSupportTicketCallback, std::move(callback)),
      email, subject, body, subscriber_credential.value_or(""),
      GetTimeZoneName());
}

void BraveVpnServiceImpl::GetSupportData(GetSupportDataCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run({}, {}, {}, {});
}

void BraveVpnServiceImpl::ResetConnectionState() {
  NOTIMPLEMENTED();
}

void BraveVpnServiceImpl::EnableOnDemand(bool /*enable*/) {
  NOTIMPLEMENTED();
}

void BraveVpnServiceImpl::GetOnDemandState(GetOnDemandStateCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(false, false);
}

void BraveVpnServiceImpl::EnableSmartProxyRouting(bool /*enable*/) {
  NOTIMPLEMENTED();
}

void BraveVpnServiceImpl::GetSmartProxyRoutingState(
    GetSmartProxyRoutingStateCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(false);
}

void BraveVpnServiceImpl::UpdateAgentConnection(mojom::PurchasedState state) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!agent_client_) {
    return;
  }

  switch (state) {
    case mojom::PurchasedState::PURCHASED:
      // The only state that justifies waking the agent. Policy still gets
      // checked: a profile where the VPN is disabled must not open a connection
      // at all.
      if (IsBraveVPNEnabled()) {
        agent_client_->EnsureConnected();
      }
      return;
    case mojom::PurchasedState::NOT_PURCHASED:
      // Nothing left to drive, so stop holding a session the agent would keep
      // alive for this profile. Reset() also clears any refusal verdict, so a
      // later purchase starts from a clean attempt.
      agent_client_->Reset();
      return;
    case mojom::PurchasedState::LOADING:
    case mojom::PurchasedState::SESSION_EXPIRED:
    case mojom::PurchasedState::FAILED:
    case mojom::PurchasedState::OUT_OF_CREDENTIALS:
      // Deliberately no change. All four are recoverable and can coexist with a
      // live tunnel, so an existing connection stays up - the person still has
      // to be able to disconnect - while none of them is a reason to open one.
      return;
  }
}

void BraveVpnServiceImpl::OnAgentConnected() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  VLOG(1) << "Agent session established";
}

void BraveVpnServiceImpl::OnAgentDisconnected() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  VLOG(1) << "Agent session lost";
}

void BraveVpnServiceImpl::OnAgentUnavailable(
    std::optional<mojom::BrowserAuthResult> result) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LOG(ERROR) << "Agent refused this browser, reason: "
             << BrowserAuthResultToString(result);
}

void BraveVpnServiceImpl::OnAgentNotRunning() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LOG(ERROR) << "Agent is not running";

  // TODO(https://github.com/brave/brave-browser/issues/58243)
  // The agent process is apparently not running. While the agent client will
  // retry on its own, check if the agent is running and start it if not. This
  // is a workaround for the case where the agent is not started by the
  // privileged helper.
}

void BraveVpnServiceImpl::SetConnectionStateForTesting(  // IN-TEST
    mojom::ConnectionState state) {
  connection_state_ = state;
  NotifyConnectionStateChanged(state);
}

void BraveVpnServiceImpl::SetPurchasedStateForTesting(  // IN-TEST
    const std::string& env,
    mojom::PurchasedState state) {
  purchased_state_manager_->SetPurchasedState(env, state);
}

}  // namespace brave_vpn::v2
