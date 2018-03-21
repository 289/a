#ifndef __PF_PATH_TRACK_H__
#define __PF_PATH_TRACK_H__

#include <stdint.h>
#include <vector>
#include "point.h"
#include "types.h"

namespace shared {
namespace net {
class Buffer;
}; // namespace shared
}; // namespace net

namespace pathfinder
{

class PathTrack
{
private:
	PATH _path;
	int32_t _index;

public:
	PathTrack();
	~PathTrack();
	PathTrack(const PathTrack& rhs);
	PathTrack& operator = (const PathTrack& rhs);

public:
	void Load(shared::net::Buffer& buffer);
	void Save(shared::net::Buffer& buffer);

	void Init(const Path& path);
	void Init(const PATH& path);
	void Clear();

	void    SetIndex(int32_t index);
	int32_t GetIndex() const;
	size_t  GetLength() const;
	const PATH& GetPath() const;
	void GetEndPos(PointF& pos) const;

	bool Append(const Path& path);
	bool Append(const PATH& path);
	bool Append(const PathTrack* path);

	/**
	 * @brief 重置路径起点为pt
	 * @param pt 路径的新起点
	 * @ret   成功返回true，失败返回false
	 */
	bool SetCurPos(PointF& pt);

	/**
	 * @brief 沿着路径从起点向前移动step长度，更新路径起点为新到达的点，删除已经走过的路径。
	 *        如果路径长度没有step那么长，走完整条路径即可，走完路径后，保留路径中最后一个点。
	 * @param next 返回新路径的起点
	 * @param step 希望走的长度
	 * @ret   返回实际走的长度，失败返回-1
	 */
	float GetNextPos(PointF& next, float step);

	/**
	 * @brief 沿着路径从头走到尾
	 * @param path 根据行走速度计算得到的路径
	 * @param step 每步的步长，即行驶速度
	 * @ret   成功返回true，失败返回false
	 */
	bool GoAcrossPath(std::vector<PointF>& path, float step);
};

typedef std::vector<PathTrack*> PathVec;

};

#endif // __PF_PATH_TRACK_H__
