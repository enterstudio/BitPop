// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_GEOLOCATION_ARBITRATOR_DEPENDENCY_FACTORIES_FOR_TEST_H_
#define CONTENT_BROWSER_GEOLOCATION_ARBITRATOR_DEPENDENCY_FACTORIES_FOR_TEST_H_

#include "content/browser/geolocation/arbitrator_dependency_factory.h"

class GeolocationArbitratorDependencyFactoryWithLocationProvider
    : public DefaultGeolocationArbitratorDependencyFactory {
 public:
  typedef LocationProviderBase* (*LocationProviderFactoryFunction)(void);

  GeolocationArbitratorDependencyFactoryWithLocationProvider(
      LocationProviderFactoryFunction factory_function);

  virtual LocationProviderBase* NewNetworkLocationProvider(
      content::AccessTokenStore* access_token_store,
      net::URLRequestContextGetter* context,
      const GURL& url,
      const string16& access_token) OVERRIDE;

  virtual LocationProviderBase* NewSystemLocationProvider() OVERRIDE;

 protected:
  virtual ~GeolocationArbitratorDependencyFactoryWithLocationProvider();

  LocationProviderFactoryFunction factory_function_;
};


#endif  // CONTENT_BROWSER_GEOLOCATION_ARBITRATOR_DEPENDENCY_FACTORIES_FOR_TEST_H_
