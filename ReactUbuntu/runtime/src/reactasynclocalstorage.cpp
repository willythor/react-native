
/**
 * Copyright (C) 2016, Canonical Ltd.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 * Author: Justin McPherson <justin.mcpherson@canonical.com>
 *
 */

#include <iterator>
#include <algorithm>

#include <QGuiApplication>
#include <QStandardPaths>
#include <QSettings>

#include <QDebug>

#include "reactasynclocalstorage.h"


namespace {
// TODO: promote to global
QVariantMap makeError(const QString& message, const QVariantMap& extraData = QVariantMap{}) {
  QVariantMap rc(extraData);
  rc.insert("message", message);
  return rc;
}
}


void ReactAsyncLocalStorage::multiGet(
  const QList<QString>& keys,
  const ReactModuleInterface::ListArgumentBlock& callback
) {
  if (m_settings == nullptr) {
    callback(m_bridge, QVariantList{
      QVariantList{makeError("Could not open storage file")}, QVariant()});
    return;
  }

  QVariantList rc;
  for (const QString& key : keys) {
    rc.push_back(QVariantList{key, m_settings->value(key)});
  }
  callback(m_bridge, QVariantList{QVariant(), rc});
}

void ReactAsyncLocalStorage::multiSet(
      const QList<QList<QString>>& kvPairs,
      const ReactModuleInterface::ListArgumentBlock& callback
) {
  if (m_settings == nullptr) {
    callback(m_bridge, QVariantList{
      QVariantList{makeError("Could not open storage file")}, QVariant()});
    return;
  }

  for (const QList<QString>& pair : kvPairs) {
    m_settings->setValue(pair.first(), pair.last());
  }
  callback(m_bridge, QVariantList{QVariant()});
}

void ReactAsyncLocalStorage::multiRemove(
      const QList<QString>& keys,
      const ReactModuleInterface::ListArgumentBlock& callback
) {
  if (m_settings == nullptr) {
    callback(m_bridge, QVariantList{
      QVariantList{makeError("Could not open storage file")}, QVariant()});
    return;
  }

  for (const QString& key : keys) {
    m_settings->remove(key);
  }
  callback(m_bridge, QVariantList{QVariant()});
}

void ReactAsyncLocalStorage::clear(
      const ReactModuleInterface::ListArgumentBlock& callback
) {
  if (m_settings == nullptr) {
    callback(m_bridge, QVariantList{
      QVariantList{makeError("Could not open storage file")}, QVariant()});
    return;
  }

  m_settings->clear();
  callback(m_bridge, QVariantList{QVariant()});
}

void ReactAsyncLocalStorage::getAllKeys(
      const ReactModuleInterface::ListArgumentBlock& callback
) {
  if (m_settings == nullptr) {
    callback(m_bridge, QVariantList{
      QVariantList{makeError("Could not open storage file")}, QVariant()});
    return;
  }

  QStringList sk = m_settings->allKeys();
  QVariantList rc;
  std::transform(sk.begin(), sk.end(), std::back_inserter(rc), [](const QString& v) {
    return QVariant::fromValue(v);
  });
  callback(m_bridge, QVariantList{QVariant(), rc});
}


ReactAsyncLocalStorage::ReactAsyncLocalStorage(QObject* parent)
  : QObject(parent)
  , m_bridge(nullptr)
  , m_settings(nullptr)
{
}

ReactAsyncLocalStorage::~ReactAsyncLocalStorage()
{
}


void ReactAsyncLocalStorage::setBridge(ReactBridge* bridge)
{
  m_bridge = bridge;

  // Setup like state saver
  QString applicationName(qApp->applicationName());
  if (applicationName.isEmpty()) {
      qCritical() << __PRETTY_FUNCTION__ << "Cannot create local storage file, application name not defined.";
      return;
  }
  QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
  if (runtimeDir.isEmpty()) {
      runtimeDir = qgetenv("XDG_RUNTIME_DIR");
  }
  if (runtimeDir.isEmpty()) {
      qCritical() << __PRETTY_FUNCTION__ << "No XDG_RUNTIME_DIR path set, cannot create local storage file.";
      return;
  }
  m_settings = new QSettings(QString("%1/%2/react-state-store").arg(runtimeDir).arg(applicationName),
                            QSettings::NativeFormat,
                            this);
  m_settings->setFallbacksEnabled(false);
}

ReactViewManager* ReactAsyncLocalStorage::viewManager()
{
  return nullptr;
}

QString ReactAsyncLocalStorage::moduleName()
{
  return "RCTAsyncLocalStorage";
}

QList<ReactModuleMethod*> ReactAsyncLocalStorage::methodsToExport()
{
  return QList<ReactModuleMethod*>{};
}

QVariantMap ReactAsyncLocalStorage::constantsToExport()
{
  return QVariantMap{};
}

