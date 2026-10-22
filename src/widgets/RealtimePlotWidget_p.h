#pragma once

namespace FluentQt::Private {

// Samples per y-range block. The data code builds the blocks and the paint code reads them, so
// both have to use the same size.
constexpr int kPlotBlockSize = 256;

} // namespace FluentQt::Private
