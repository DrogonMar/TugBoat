#pragma once

#define TB_CLASS() \
public:            \
	static TB_API int64_t GetInstanceCount() {return s_InstanceCount;}\
private: \
	static TB_API int64_t s_InstanceCount;

#define TB_CLASS_IMPL(classname) int64_t classname::s_InstanceCount = 0 ;

#define TB_CLASS_CONSTRUCT() s_InstanceCount++;

#define TB_CLASS_DESTRUCT() s_InstanceCount--;