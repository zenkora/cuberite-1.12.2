#pragma once

#include "../CraftingRecipes.h"
#include <optional>

/**
The RecipeMapper handles the translation of crafting recipes into protocol
specific recipe Ids.
The crafting recipes are identified by the RecipeId.
The actual configuration is stored in the protocol specific configuration
directory, e.g. `Server/Protocol/1.12.2/base.recipes.txt`
*/
class cRecipeMapper
{
public:
	cRecipeMapper(void);
	~cRecipeMapper();

	/** Translates the cuberite RecipeId to the protocol specific RecipeId */
	std::optional<UInt32> GetProtocolRecipeId(UInt32 a_RecipeId);

	/** Translates the protocol specific RecipeId to the cuberite RecipeId */
	std::optional<UInt32> GetCuberiteRecipeId(UInt32 a_ProtocolRecipeId);

private:
	/** A mapping from the protocol specific RecipeId and the cuberite RecipeId */
	std::map<UInt32, UInt32> m_RecipeMap;

	/** Load Recipes from the protocol specific mapping file */
	void loadRecipes();

	/** Handles a single line of the protocol specific mapping file */
	void AddRecipeLine(int a_LineNum, const AString & a_RecipeLine, const std::map<AString, UInt32> & a_RecipeNameMap);
};