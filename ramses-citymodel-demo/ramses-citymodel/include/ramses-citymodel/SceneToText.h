//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef SCENETOTEXT_H
#define SCENETOTEXT_H

#include <string>
#include <sstream>
#include <unordered_set>

namespace ramses
{
    class Scene;
    class RamsesClient;
    class Node;
    class MeshNode;
    class RenderGroup;
}

using NodeSet = std::unordered_set<ramses::Node const*>;

class SceneToText
{
public:
    SceneToText(const ramses::Scene& scene, const ramses::RamsesClient& client, bool printTransformations);

    void printToStream(std::ostringstream& stream) const;

private:
    void printRenderPasses(std::ostringstream& stream) const;
    void printSubtree(std::ostringstream& stream, const ramses::Node& rootNode, uint32_t indentation) const;
    void printGroups(std::ostringstream& stream, const ramses::RenderGroup& group, uint32_t indentation) const;
    void printTransformations(std::ostringstream& stream, const ramses::Node& node, uint32_t indentation) const;
    NodeSet collectRootNodes() const;

    const ramses::Scene& m_scene;
    const ramses::RamsesClient& m_client;
    bool m_printTransformations;
};

#endif
