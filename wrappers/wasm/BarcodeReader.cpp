/*
* Copyright 2016 Nu-book Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ReadBarcode.h"

#include <string>
#include <memory>
#include <stdexcept>
#include <emscripten/bind.h>

struct ReadResult
{
	std::string format;
	std::vector<uint8_t> rawBytes;
	std::wstring text;
	std::string error;
	ZXing::Position position;
};

ReadResult readBarcodeFromPixmap(int bufferPtr, int imgWidth, int imgHeight, bool tryHarder, std::string format)
{
	using namespace ZXing;
	try {
		DecodeHints hints;
		hints.setTryHarder(tryHarder);
		hints.setTryRotate(tryHarder);
		hints.setFormats(BarcodeFormatsFromString(format));

		auto result =
			ReadBarcode({reinterpret_cast<uint8_t*>(bufferPtr), imgWidth, imgHeight, ImageFormat::RGBX}, hints);

		if (result.isValid()) {
			return { ToString(result.format()), result.rawBytes(), result.text(),  };
		}
	}
	catch (const std::exception& e) {
		return { "", {}, L"", e.what() };
	}
	catch (...) {
		return { "", {}, L"", "Unknown error" };
	}
	return {};
}

EMSCRIPTEN_BINDINGS(BarcodeReader)
{
	using namespace emscripten;

	register_vector<uint8_t>("ByteArray");

	value_object<ReadResult>("ReadResult")
			.field("format", &ReadResult::format)
			.field("rawBytes", &ReadResult::rawBytes)
			.field("text", &ReadResult::text)
			.field("error", &ReadResult::error)
			.field("position", &ReadResult::position)
			;

	value_object<ZXing::PointI>("Point")
			.field("x", &ZXing::PointI::x)
			.field("y", &ZXing::PointI::y)
			;

	value_object<ZXing::Position>("Position")
			.field("topLeft", emscripten::index<0>())
			.field("topRight", emscripten::index<1>())
			.field("bottomRight", emscripten::index<2>())
			.field("bottomLeft", emscripten::index<3>())
			;

	function("readBarcodeFromPixmap", &readBarcodeFromPixmap);
}
