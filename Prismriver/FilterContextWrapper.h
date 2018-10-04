#pragma once

#include "CodecContextWrapper.h"

extern "C"
{
#include <libavfilter/buffersink.h>
}

namespace Prismriver
{
	private class FilterContextWrapper : public CodecContextWrapper
	{
	private:
		// No default or copy constructor or assign operator
		FilterContextWrapper() = delete;
		FilterContextWrapper(const FilterContextWrapper& other) = delete;
		FilterContextWrapper& operator=(const FilterContextWrapper& other) = delete;

	public:
		FilterContextWrapper(Stream^ stream) : CodecContextWrapper(stream) {}
		FilterContextWrapper(String^ file) : CodecContextWrapper(file) {}

		virtual ~FilterContextWrapper();

		/**
		 * The AVFilterGraph for this FilterContextWrapper.
		 */
		AVFilterGraph* filterGraph;
		/**
		 * The AVFilterContext for this FilterContextWrapper.
		 */
		AVFilterContext* bufferSinkContext;
		/**
		 * The AVFilterContext for this FilterContextWrapper.
		 */
		AVFilterContext* bufferSourceContext;

		void InitFilters(CodecContextWrapper* dataIn);
	};

}