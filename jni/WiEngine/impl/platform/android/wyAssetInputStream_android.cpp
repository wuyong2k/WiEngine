/*
 * Copyright (c) 2010 WiYun Inc.
 * Author: luma(stubma@gmail.com)
 *
 * For all entities this program is free software; you can redistribute
 * it and/or modify it under the terms of the 'WiEngine' license with
 * the additional provision that 'WiEngine' must be credited in a manner
 * that can be be observed by end users, for example, in the credits or during
 * start up. (please find WiEngine logo in sdk's logo folder)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#if ANDROID

#include "wyAssetInputStream_android.h"
#include "wyInit.h"
#include "wyLog.h"
#include <errno.h>
#include "wyGlobal.h"

// aal
extern wyAAL gAAL;

wyAssetInputStream* wyAssetInputStream::make(int resId) {
	wyAssetInputStream* ais = WYNEW wyAssetInputStream_android(resId);
	return (wyAssetInputStream*)ais->autoRelease();
}

wyAssetInputStream* wyAssetInputStream::make(const char* path, bool isFile) {
	wyAssetInputStream* ais = WYNEW wyAssetInputStream_android(path, isFile);
	return (wyAssetInputStream*)ais->autoRelease();
}

wyAssetInputStream_android::wyAssetInputStream_android(int resId) :
		wyAssetInputStream(resId),
		m_asset(NULL),
		m_fp(NULL) {
	m_asset = gAAL.getAssetByResId(resId, NULL);
}

wyAssetInputStream_android::wyAssetInputStream_android(const char* path, bool isFile) :
		wyAssetInputStream(path, isFile),
		m_asset(NULL),
		m_fp(NULL) {
	if(isFile) {
		// open file
		if((m_fp = fopen(path, "rb")) == NULL) {
			LOGW("open file %s failed: %s", path, strerror(errno));
			m_fp = NULL;
		}
	} else {
		m_asset = gAAL.getAsset(path);
	}
}

wyAssetInputStream_android::~wyAssetInputStream_android() {
	if(m_asset != NULL) {
		gAAL.closeAsset(m_asset);
		m_asset = NULL;
	} else if(m_fp != NULL) {
		fclose(m_fp);
		m_fp = NULL;
	}
}

size_t wyAssetInputStream_android::getLength() {
	if(m_asset != NULL)
		return gAAL.getAssetLength(m_asset);
	else if(m_fp != NULL) {
		size_t offset = ftell(m_fp);
		fseek(m_fp, 0, SEEK_END);
		size_t len = ftell(m_fp);
		fseek(m_fp, offset, SEEK_SET);
		return len;
	} else {
		return 0;
	}
}

size_t wyAssetInputStream_android::getPosition() {
	if(m_asset != NULL)
		return gAAL.getAssetLength(m_asset) - gAAL.getAssetRemainingLength(m_asset);
	else if(m_fp != NULL)
		return ftell(m_fp);
	else
		return 0;
}

size_t wyAssetInputStream_android::available() {
	if(m_asset != NULL)
		return gAAL.getAssetRemainingLength(m_asset);
	else if(m_fp != NULL)
		return getLength() - getPosition();
	else
		return 0;
}

char* wyAssetInputStream_android::getBuffer() {
	size_t len = getLength();
	char* buf = (char*)wyMalloc(len * sizeof(char));

	if(m_asset != NULL)
		memcpy(buf, gAAL.getAssetBuffer(m_asset), len);
	else if(m_fp != NULL)
		fread(buf, sizeof(char), len, m_fp);

	return buf;
}

void wyAssetInputStream_android::close() {
	if(m_asset != NULL) {
		gAAL.closeAsset(m_asset);
		m_asset = NULL;
	} else if(m_fp != NULL) {
		fclose(m_fp);
		m_fp = NULL;
	}
}

ssize_t wyAssetInputStream_android::read(char* buffer, size_t length) {
	if(m_asset != NULL)
		return gAAL.readAsset(m_asset, buffer, length);
	else if(m_fp != NULL)
		return fread(buffer, sizeof(char), length, m_fp);
	else
		return -1;
}

size_t wyAssetInputStream_android::seek(int offset, int mode) {
	if(m_asset != NULL) {
		return gAAL.seekAsset(m_asset, offset, mode);
	} else if(m_fp != NULL) {
		fseek(m_fp, offset, mode);
		return ftell(m_fp);
	} else {
		return 0;
	}
}

#endif // #if ANDROID
