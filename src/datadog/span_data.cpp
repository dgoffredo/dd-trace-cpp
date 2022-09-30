#include "span_data.h"

#include <cstddef>
#include <exception>
#include <string_view>

#include "error.h"
#include "msgpack.h"
#include "span_config.h"
#include "span_defaults.h"
#include "tags.h"

namespace datadog {
namespace tracing {
namespace {

std::optional<std::string_view> lookup(
    const std::string& key,
    const std::unordered_map<std::string, std::string>& map) {
  const auto found = map.find(key);
  if (found != map.end()) {
    return found->second;
  }
  return std::nullopt;
}

}  // namespace

std::optional<std::string_view> SpanData::environment() const {
  return lookup(tags::environment, tags);
}

std::optional<std::string_view> SpanData::version() const {
  return lookup(tags::version, tags);
}

void SpanData::apply_config(const SpanDefaults& defaults,
                            const SpanConfig& config, const Clock& clock) {
  service = config.service.value_or(defaults.service);
  name = config.name.value_or(defaults.name);

  tags = defaults.tags;
  std::string environment = config.environment.value_or(defaults.environment);
  if (!environment.empty()) {
    tags.insert_or_assign(tags::environment, environment);
  }
  std::string version = config.version.value_or(defaults.version);
  if (!version.empty()) {
    tags.insert_or_assign(tags::version, version);
  }
  for (const auto& [key, value] : config.tags) {
    if (!tags::is_internal(key)) {
      tags.insert_or_assign(key, value);
    }
  }

  resource = config.resource.value_or(name);
  service_type = config.service_type.value_or(defaults.service_type);
  if (config.start) {
    start = *config.start;
  } else {
    start = clock();
  }
}

Expected<void> msgpack_encode(std::string& destination,
                              const SpanData& span) try {
  // Be sure to update `num_fields` when adding fields.
  const std::size_t num_fields = 12;
  msgpack::pack_map(destination, num_fields);

  msgpack::pack_string(destination, "service");
  msgpack::pack_string(destination, span.service);

  msgpack::pack_string(destination, "name");
  msgpack::pack_string(destination, span.name);

  msgpack::pack_string(destination, "resource");
  msgpack::pack_string(destination, span.resource);

  msgpack::pack_string(destination, "trace_id");
  msgpack::pack_integer(destination, span.trace_id);

  msgpack::pack_string(destination, "span_id");
  msgpack::pack_integer(destination, span.span_id);

  msgpack::pack_string(destination, "parent_id");
  msgpack::pack_integer(destination, span.parent_id);

  msgpack::pack_string(destination, "start");
  msgpack::pack_integer(destination,
                        std::chrono::duration_cast<std::chrono::nanoseconds>(
                            span.start.wall.time_since_epoch())
                            .count());

  msgpack::pack_string(destination, "duration");
  msgpack::pack_integer(
      destination,
      std::chrono::duration_cast<std::chrono::nanoseconds>(span.duration)
          .count());

  msgpack::pack_string(destination, "error");
  msgpack::pack_integer(destination, std::int32_t(span.error));

  msgpack::pack_string(destination, "meta");
  msgpack::pack_map(destination, span.tags,
                    [](std::string& destination, const auto& value) {
                      msgpack::pack_string(destination, value);
                    });

  msgpack::pack_string(destination, "metrics");
  msgpack::pack_map(destination, span.numeric_tags,
                    [](std::string& destination, const auto& value) {
                      msgpack::pack_double(destination, value);
                    });

  msgpack::pack_string(destination, "type");
  msgpack::pack_string(destination, span.service_type);

  // Be sure to update `num_fields` when adding fields.
  return std::nullopt;
} catch (const std::exception& error) {
  return Error{Error::MESSAGEPACK_ENCODE_FAILURE, error.what()};
}

}  // namespace tracing
}  // namespace datadog