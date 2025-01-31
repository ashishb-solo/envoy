#pragma once

#include "envoy/http/filter.h"
#include "envoy/matcher/matcher.h"
#include "envoy/server/factory_context.h"
#include "envoy/type/matcher/v3/http_inputs.pb.h"
#include "envoy/type/matcher/v3/http_inputs.pb.validate.h"

#include "source/common/http/header_utility.h"

namespace Envoy {
namespace Http {
namespace Matching {
/**
 * Common base class for all the header/trailer DataInputs.
 */
template <class HeaderType>
class HttpHeadersDataInputBase : public Matcher::DataInput<HttpMatchingData> {
public:
  explicit HttpHeadersDataInputBase(const std::string& name) : name_(name) {}

  virtual OptRef<const HeaderType> headerMap(const HttpMatchingData& data) const PURE;

  Matcher::DataInputGetResult get(const HttpMatchingData& data) const override {
    const OptRef<const HeaderType> maybe_headers = headerMap(data);

    if (!maybe_headers) {
      return {Matcher::DataInputGetResult::DataAvailability::NotAvailable, absl::nullopt};
    }

    auto header_string = HeaderUtility::getAllOfHeaderAsString(*maybe_headers, name_, ",");

    if (header_string.result()) {
      return {Matcher::DataInputGetResult::DataAvailability::AllDataAvailable,
              std::string(header_string.result().value())};
    }

    return {Matcher::DataInputGetResult::DataAvailability::AllDataAvailable, absl::nullopt};
  }

private:
  const LowerCaseString name_;
};

/**
 * Common base class for all the header/trailer DataInputsFactory.
 */
template <class DataInputType, class ProtoType>
class HttpHeadersDataInputFactoryBase : public Matcher::DataInputFactory<HttpMatchingData> {
public:
  explicit HttpHeadersDataInputFactoryBase(const std::string& name) : name_(name) {}

  std::string name() const override { return "envoy.matching.inputs." + name_; }

  Matcher::DataInputFactoryCb<HttpMatchingData>
  createDataInputFactoryCb(const Protobuf::Message& config,
                           ProtobufMessage::ValidationVisitor& validation_visitor) override {
    const auto& typed_config =
        MessageUtil::downcastAndValidate<const ProtoType&>(config, validation_visitor);

    return [header_name = typed_config.header_name()] {
      return std::make_unique<DataInputType>(header_name);
    };
  };
  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return std::make_unique<ProtoType>();
  }

private:
  const std::string name_;
};

class HttpRequestHeadersDataInput : public HttpHeadersDataInputBase<RequestHeaderMap> {
public:
  explicit HttpRequestHeadersDataInput(const std::string& name) : HttpHeadersDataInputBase(name) {}

  OptRef<const RequestHeaderMap> headerMap(const HttpMatchingData& data) const override {
    return data.requestHeaders();
  }
};

class HttpRequestHeadersDataInputFactory
    : public HttpHeadersDataInputFactoryBase<
          HttpRequestHeadersDataInput, envoy::type::matcher::v3::HttpRequestHeaderMatchInput> {
public:
  HttpRequestHeadersDataInputFactory() : HttpHeadersDataInputFactoryBase("request_headers") {}
};

class HttpResponseHeadersDataInput : public HttpHeadersDataInputBase<ResponseHeaderMap> {
public:
  explicit HttpResponseHeadersDataInput(const std::string& name) : HttpHeadersDataInputBase(name) {}

  OptRef<const ResponseHeaderMap> headerMap(const HttpMatchingData& data) const override {
    return data.responseHeaders();
  }
};

class HttpResponseHeadersDataInputFactory
    : public HttpHeadersDataInputFactoryBase<
          HttpResponseHeadersDataInput, envoy::type::matcher::v3::HttpResponseHeaderMatchInput> {
public:
  HttpResponseHeadersDataInputFactory() : HttpHeadersDataInputFactoryBase("response_headers") {}
};

class HttpRequestTrailersDataInput : public HttpHeadersDataInputBase<RequestTrailerMap> {
public:
  explicit HttpRequestTrailersDataInput(const std::string& name) : HttpHeadersDataInputBase(name) {}

  OptRef<const RequestTrailerMap> headerMap(const HttpMatchingData& data) const override {
    return data.requestTrailers();
  }
};

class HttpRequestTrailersDataInputFactory
    : public HttpHeadersDataInputFactoryBase<
          HttpRequestTrailersDataInput, envoy::type::matcher::v3::HttpRequestTrailerMatchInput> {
public:
  HttpRequestTrailersDataInputFactory() : HttpHeadersDataInputFactoryBase("request_trailers") {}
};

class HttpResponseTrailersDataInput : public HttpHeadersDataInputBase<ResponseTrailerMap> {
public:
  explicit HttpResponseTrailersDataInput(const std::string& name)
      : HttpHeadersDataInputBase(name) {}

  OptRef<const ResponseTrailerMap> headerMap(const HttpMatchingData& data) const override {
    return data.responseTrailers();
  }
};

class HttpResponseTrailersDataInputFactory
    : public HttpHeadersDataInputFactoryBase<
          HttpResponseTrailersDataInput, envoy::type::matcher::v3::HttpResponseTrailerMatchInput> {
public:
  HttpResponseTrailersDataInputFactory() : HttpHeadersDataInputFactoryBase("response_trailers") {}
};
} // namespace Matching
} // namespace Http
} // namespace Envoy
