// PatchOrchestrator — Sprint 3 (A3) shared app state.
//
// See demo_app_context.hpp for the contract. Defaults: empty schedule id, the
// standard local API base URL, and an initial "idle" rollout state.

#include "demo_app_context.hpp"

namespace {

const char kDefaultApiBaseUrl[] = "http://localhost:5000";
const char kInitialRolloutState[] = "idle";

}  // namespace

DemoAppContext::DemoAppContext(QObject *parent)
    : QObject(parent)
    , m_scheduleId()
    , m_apiBaseUrl(QLatin1String(kDefaultApiBaseUrl))
    , m_rolloutState(QLatin1String(kInitialRolloutState))
{
}

void DemoAppContext::setScheduleId(const QString &id)
{
    if (m_scheduleId == id)
        return;
    m_scheduleId = id;
    emit scheduleIdChanged(m_scheduleId);
}

void DemoAppContext::setApiBaseUrl(const QString &url)
{
    if (m_apiBaseUrl == url)
        return;
    m_apiBaseUrl = url;
    emit apiBaseUrlChanged(m_apiBaseUrl);
}

void DemoAppContext::setRolloutState(const QString &state)
{
    if (m_rolloutState == state)
        return;
    m_rolloutState = state;
    emit rolloutStateChanged(m_rolloutState);
}
