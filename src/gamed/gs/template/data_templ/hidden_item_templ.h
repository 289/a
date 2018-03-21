#ifndef GAMED_GS_TEMPLATE_DATATEMPL_HIDDEN_ITEM_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_HIDDEN_ITEM_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl
{

/*
 * @brief: 隐藏物品模板
 * @brief: 需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 */
class HiddenItemTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(HiddenItemTempl, TEMPL_TYPE_HIDDEN_ITEM);

	struct param_value
	{
		enum 
		{
			VAR_IS_INT    = 1,
			VAR_IS_DOUBLE = 2,
		};

		union arithmeticType
		{
			int64_t i_var;
			double  d_var;
		};

		void Pack(shared::net::ByteBuffer& buf)
		{
			buf << var_type;
			if (var_type == VAR_IS_INT)
				buf << value.i_var;
			else if (var_type == VAR_IS_DOUBLE)
				buf << value.d_var;
			else
				assert(false);
		}

		void UnPack(shared::net::ByteBuffer& buf)
		{
			buf >> var_type;
			if (var_type == VAR_IS_INT)
				buf >> value.i_var;
			else if (var_type == VAR_IS_DOUBLE)
				buf >> value.d_var;
			else
				assert(false);
		}

		param_value() : var_type(0) { }

		uint8_t        var_type;
		arithmeticType value;
	};

	static const size_t kMaxParamCount = 8;
	std::vector<param_value> param_list; // 参数列表，不能暴露出来直接使用

public:
	enum HiddenItemType
	{
		HIT_INVALID = 0,
		HIT_TEAM_NPC,     // param: TeamNpcParam
		HIT_LABEL_ITEM,   // 用做标记的物品，逻辑和简单物品、任务物品一致。没有参数
	};

	struct TeamNpcParam
	{
		int64_t monster_tid;
	};


public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

// 0
	int32_t type; // 隐藏物品的类型（黑盒类型）,对应枚举HiddenItemType

	///
	/// 设置参数
	///
	void SetParam(int64_t iparam)
	{
		if (param_list.size() >= kMaxParamCount)
		{
			assert(false);
			return;
		}

		param_value tmp;
		tmp.var_type    = param_value::VAR_IS_INT;
		tmp.value.i_var = iparam;
		param_list.push_back(tmp);
	}

	void SetParam(double dparam)
	{
		if (param_list.size() >= kMaxParamCount)
		{
			assert(false);
			return;
		}

		param_value tmp;
		tmp.var_type    = param_value::VAR_IS_DOUBLE;
		tmp.value.d_var = dparam;
		param_list.push_back(tmp);
	}

	///
	/// 获取参数
	///
	class PopParamGuard
	{
	public:
		PopParamGuard(const HiddenItemTempl& templ)
			: templ_(templ),
			  index_(0)
		{ }

		~PopParamGuard()
		{
			index_  = 0;
		}

		bool PopParam(int64_t& iparam)
		{
			if (index_ >= templ_.param_list.size())
				return false;

			param_value tmpvalue = templ_.param_list[index_];
			if (tmpvalue.var_type != param_value::VAR_IS_INT)
				return false;

			iparam = tmpvalue.value.i_var;
			++index_;
			return true;
		}

		bool PopParam(double& dparam)
		{
			if (index_ >= templ_.param_list.size())
				return false;

			param_value tmpvalue = templ_.param_list[index_];
			if (tmpvalue.var_type != param_value::VAR_IS_DOUBLE)
				return false;

			dparam = tmpvalue.value.d_var;
			++index_;
			return true;
		}

	private:
		const HiddenItemTempl& templ_;
		size_t index_;
	};


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(type, param_list);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(type, param_list);
	}

	bool CheckParamByIndex(size_t index, int64_t& iparam) const
	{
		if (index >= param_list.size())
			return false;

		param_value tmpvalue = param_list[index];
		if (tmpvalue.var_type != param_value::VAR_IS_INT)
			return false;

		iparam = tmpvalue.value.i_var;
		return true;
	}

	bool CheckParamByIndex(size_t index, double& dparam) const
	{
		if (index >= param_list.size())
			return false;

		param_value tmpvalue = param_list[index];
		if (tmpvalue.var_type != param_value::VAR_IS_DOUBLE)
			return false;

		dparam = tmpvalue.value.d_var;
		return true;
	}

	virtual bool OnCheckDataValidity() const
	{
		if (type == HIT_TEAM_NPC)
		{
			if (param_list.size() != 1)
				return false;

			TeamNpcParam param;
			if (CheckParamByIndex(0, param.monster_tid))
			{
				if (param.monster_tid <= 0)
					return false;
			}
			else
			{
				return false;
			}
		}
		else if (type == HIT_LABEL_ITEM)
		{
			if (param_list.size() != 0)
				return false;
		}
		else
		{
			return false;
		}

		return true;
	}
}; 

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_HIDDEN_ITEM_TEMPL_H_
