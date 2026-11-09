#include "Globals.h"
#include "RecipeMapper.h"
#include "../Root.h"

cRecipeMapper::cRecipeMapper(void)
{
	loadRecipes();
}


void cRecipeMapper::loadRecipes()
{
	cFile f;
	if (!f.Open("Protocol/1.12.2/base.recipes.txt", cFile::fmRead))
	{
		LOGWARNING("Cannot open file \"Protocol/1.12.2/base.recipes.txt\", no recipe book recipes will be available!");
		return;
	}
	AString Everything;
	if (!f.ReadRestOfFile(Everything))
	{
		LOGWARNING("Cannot read file \"Protocol/1.12.2/base.recipes.txt\", no recipe book recipes will be available!");
		return;
	}
	f.Close();

	// Split it into lines, then process each line as a single recipe:
	AStringVector Split = StringSplit(Everything, "\n");
	const auto & RecipeNameMap = cRoot::Get()->GetCraftingRecipes()->GetRecipeNameMap();

	int LineNum = 1;
	for (AStringVector::const_iterator itr = Split.begin(); itr != Split.end(); ++itr, ++LineNum)
	{
		// Remove anything after a '#' sign and trim away the whitespace:
		AString Recipe = TrimString(itr->substr(0, itr->find('#')));
		if (Recipe.empty())
		{
			// Empty recipe
			continue;
		}
		AddRecipeLine(LineNum, Recipe, RecipeNameMap);
	}
	LOG("Loaded %zu recipe book entries", m_RecipeMap.size());
}


void cRecipeMapper::AddRecipeLine(int a_LineNum, const AString & a_RecipeLine, const std::map<AString, UInt32> & a_RecipeNameMap)
{
	AStringVector Sides = StringSplit(a_RecipeLine, " ");
	UInt32 Id;
	if (Sides.size() != 2)
	{
		LOGINFO("Recipe incompletely configured %s", a_RecipeLine);
		return;
	}
	StringToInteger<UInt32>(Sides[0], Id);

	auto RecipeIndex = a_RecipeNameMap.find(Sides[1]);
	if (RecipeIndex == a_RecipeNameMap.end())
	{
		return;
	}
	m_RecipeMap.emplace(Id, RecipeIndex->second);
}


cRecipeMapper::~cRecipeMapper()
{
}


std::optional<UInt32> cRecipeMapper::GetProtocolRecipeId(UInt32 a_RecipeId)
{
	for (const auto & item: m_RecipeMap)
	{
		if (item.second == a_RecipeId)
		{
			return item.first;
		}
	}
	return {};
}


std::optional<UInt32> cRecipeMapper::GetCuberiteRecipeId(UInt32 a_ProtocolRecipeId)
{
	auto Element = m_RecipeMap.find(a_ProtocolRecipeId);
	if (Element != m_RecipeMap.end())
	{
		return Element->second;
	}
	return {};
}