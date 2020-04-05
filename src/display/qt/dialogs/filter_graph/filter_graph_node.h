/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Software: VCS
 * 
 */

#ifndef FILTER_GRAPH_FILTER_GRAPH_NODE_H
#define FILTER_GRAPH_FILTER_GRAPH_NODE_H

#include "display/qt/subclasses/QGraphicsItem_interactible_node_graph_node.h"

class filter_c;

enum class filter_node_type_e
{
    gate,
    filter,
};

class FilterGraphNode : public QObject, public InteractibleNodeGraphNode
{
    Q_OBJECT

public:
    FilterGraphNode(const filter_node_type_e filterType,
                    const QString title,
                    const unsigned width = 240,
                    const unsigned height = 130);
    virtual ~FilterGraphNode();

    const filter_c *associatedFilter = nullptr;

    // Convenience functions that can be used to access the node's (default) input and output edge.
    virtual node_edge_s* input_edge(void) { return nullptr; }
    virtual node_edge_s* output_edge(void) { return nullptr; }

    void set_background_color(const QString colorName);
    const QList<QString>& background_color_list(void);
    const QString& current_background_color_name(void);

protected:
    const filter_node_type_e filterType;

    // A list of the node background colors we support. Note: You shouldn't
    // delete or change the names of any of the items on this list. But
    // you can add more.
    const QList<QString> backgroundColorList = {
        "Blue", "Cyan", "Green", "Magenta", "Red", "Yellow",
    };

    QString backgroundColor = this->backgroundColorList.at(1);

private:
    void generate_right_click_menu(void);
};

#endif
