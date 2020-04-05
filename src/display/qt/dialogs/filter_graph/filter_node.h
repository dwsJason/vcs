/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Software: VCS
 * 
 */

#ifndef FILTER_GRAPH_FILTER_NODE_H
#define FILTER_GRAPH_FILTER_NODE_H

#include "filter/filter.h"
#include "display/qt/dialogs/filter_graph/filter_graph_node.h"

class FilterNode : public FilterGraphNode
{
public:
    FilterNode(const QString title,
               const unsigned width = 240,
               const unsigned height = 130) : FilterGraphNode(filter_node_type_e::filter, title, width, height)
    {
        this->edges =
        {
            node_edge_s(node_edge_s::direction_e::in, QRect(-10, 11, 18, 18), this),
            node_edge_s(node_edge_s::direction_e::out, QRect(this->width-10, 11, 18, 18), this),
        };

        return;
    }

    QRectF boundingRect(void) const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    node_edge_s* input_edge(void) override;
    node_edge_s* output_edge(void) override;

private:
};

#endif
