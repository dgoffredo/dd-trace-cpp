#pragma once

#include "clock.h"
#include "id_generator.h"
#include "error.h"
#include "span.h"
#include "tracer_config.h"

namespace datadog {
namespace tracing {

class DictReader;
class SpanConfig;
class TraceSampler;
class SpanSampler;

class Tracer {
    std::shared_ptr<Collector> collector_;
    std::shared_ptr<TraceSampler> trace_sampler_;
    std::shared_ptr<SpanSampler> span_sampler_;
    IDGenerator generator;
    Clock clock;
    
 public:
  explicit Tracer(const TracerConfig& config);
  Tracer(const TracerConfig& config, const IDGenerator& generator, const Clock& clock);

  std::variant<Span, Error> create_span(const SpanConfig& config);
  // ...

  std::variant<Span, Error> extract_span(const DictReader& reader);
  std::variant<Span, Error> extract_span(const DictReader& reader,
                                         const SpanConfig& config);
};

}  // namespace tracing
}  // namespace datadog
