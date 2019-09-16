/*
 * 2019 Tarpeeksi Hyvae Soft /
 * VCS disk access
 *
 * Provides functions for loading and saving data to and from files on disk.
 *
 */

#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QFile>
#include "display/qt/dialogs/filters_dialog_nodes.h"
#include "display/qt/widgets/filter_widgets.h"
#include "common/propagate.h"
#include "capture/capture.h"
#include "capture/alias.h"
#include "filter/filter.h"
#include "common/disk.h"
#include "common/csv.h"

// A wrapper for Qt's file-saving functionality. Streams given strings via the
// << operator on the object into a file.
class file_writer_c
{
public:
    file_writer_c(QString filename) :
        targetFilename(filename),
        tempFilename(targetFilename + ".temporary")
    {
        file.setFileName(tempFilename);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        stream.setDevice(&file);

        return;
    }

    // Saves the data from the temp file to the final target file.
    bool save_and_close(void)
    {
        if (QFile(targetFilename).exists() &&
            !QFile(targetFilename).remove())
        {
            return false;
        }

        file.close();

        if (!QFile::rename(tempFilename, targetFilename))
        {
            return false;
        }

        return true;
    }

    bool is_valid(void)
    {
        return isValid;
    }

    QTextStream& operator<<(const QString &string)
    {
        if (this->isValid)
        {
            stream << string;
            test_validity();
        }

        return stream;
    }

    QTextStream& operator<<(const unsigned unsignedInt)
    {
        return stream << QString::number(unsignedInt);
    }

private:
    bool isValid = true;

    QFile file;
    QTextStream stream;

    // The filename we want to save to.
    QString targetFilename;

    // The filename we'll first temporarily write the data to. If all goes well,
    // we'll rename this to the target filename, and remove the old target file,
    // if it exists.
    QString tempFilename;

    void test_validity(void)
    {
        if (!this->file.isWritable() ||
            !this->file.isOpen() ||
            this->file.error() != QFileDevice::NoError)
        {
            isValid = false;
        }

        return;
    }
};

bool kdisk_save_mode_params(const std::vector<video_mode_params_s> &modeParams,
                            const QString &targetFilename)
{
    file_writer_c outFile(targetFilename);

    // Each mode params block consists of two values specifying the resolution
    // followed by a set of string-value pairs for the different parameters.
    for (const auto &m: modeParams)
    {
        // Resolution.
        outFile << "resolution," << m.r.w << "," << m.r.h << "\n";

        // Video params.
        outFile << "vPos," << m.video.verticalPosition << "\n"
                << "hPos," << m.video.horizontalPosition << "\n"
                << "hScale," << m.video.horizontalScale << "\n"
                << "phase," << m.video.phase << "\n"
                << "bLevel," << m.video.blackLevel << "\n";

        // Color params.
        outFile << "bright," << m.color.overallBrightness << "\n"
                << "contr," << m.color.overallContrast << "\n"
                << "redBr," << m.color.redBrightness << "\n"
                << "redCn," << m.color.redContrast << "\n"
                << "greenBr," << m.color.greenBrightness << "\n"
                << "greenCn," << m.color.greenContrast << "\n"
                << "blueBr," << m.color.blueBrightness << "\n"
                << "blueCn," << m.color.blueContrast << "\n";

        // Separate the next block.
        outFile << "\n";
    }

    if (!outFile.is_valid() ||
        !outFile.save_and_close())
    {
        NBENE(("Failed to write mode params to file."));
        goto fail;
    }

    kpropagate_saved_mode_params_to_disk(modeParams, targetFilename.toStdString());

    return true;

    fail:
    kd_show_headless_error_message("Data was not saved",
                                   "An error was encountered while preparing the mode "
                                   "settings for saving. As a result, no data was saved. \n\nMore "
                                   "information about this error may be found in the terminal.");
    return false;
}

bool kdisk_load_video_mode_params(const std::string &sourceFilename)
{
    if (sourceFilename.empty())
    {
        DEBUG(("No mode settings file defined, skipping."));
        return true;
    }

    std::vector<video_mode_params_s> videoModeParams;

    QList<QStringList> paramRows = csv_parse_c(QString::fromStdString(sourceFilename)).contents();

    // Each mode is saved as a block of rows, starting with a 3-element row defining
    // the mode's resolution, followed by several 2-element rows defining the various
    // video and color parameters for the resolution.
    for (int i = 0; i < paramRows.count();)
    {
        if ((paramRows[i].count() != 3) ||
            (paramRows[i].at(0) != "resolution"))
        {
            NBENE(("Expected a 3-parameter 'resolution' statement to begin a mode params block."));
            goto fail;
        }

        video_mode_params_s p;
        p.r.w = paramRows[i].at(1).toUInt();
        p.r.h = paramRows[i].at(2).toUInt();

        i++;    // Move to the next row to start fetching the params for this resolution.

        auto get_param = [&](const QString &name)->QString
                         {
                             if (paramRows[i].at(0) != name)
                             {
                                 NBENE(("Error while loading video parameters: expected '%s' but got '%s'.",
                                        name.toLatin1().constData(), paramRows[i].at(0).toLatin1().constData()));
                                 throw 0;
                             }
                             return paramRows[i++].at(1);
                         };
        try
        {
            // Note: the order in which the params are fetched is fixed to the
            // order in which they were saved.
            p.video.verticalPosition = get_param("vPos").toInt();
            p.video.horizontalPosition = get_param("hPos").toInt();
            p.video.horizontalScale = get_param("hScale").toInt();
            p.video.phase = get_param("phase").toInt();
            p.video.blackLevel = get_param("bLevel").toInt();
            p.color.overallBrightness = get_param("bright").toInt();
            p.color.overallContrast = get_param("contr").toInt();
            p.color.redBrightness = get_param("redBr").toInt();
            p.color.redContrast = get_param("redCn").toInt();
            p.color.greenBrightness = get_param("greenBr").toInt();
            p.color.greenContrast = get_param("greenCn").toInt();
            p.color.blueBrightness = get_param("blueBr").toInt();
            p.color.blueContrast = get_param("blueCn").toInt();
        }
        catch (int)
        {
            NBENE(("Failed to load mode params from disk."));
            goto fail;
        }

        videoModeParams.push_back(p);
    }

    // Sort the modes so they'll display more nicely in the GUI.
    std::sort(videoModeParams.begin(), videoModeParams.end(), [](const video_mode_params_s &a, const video_mode_params_s &b)
                                          { return (a.r.w * a.r.h) < (b.r.w * b.r.h); });

    kpropagate_loaded_mode_params_from_disk(videoModeParams, sourceFilename);

    return true;

    fail:
    kd_show_headless_error_message("Data was not loaded",
                                   "An error was encountered while loading video parameters. "
                                   "No data was loaded.\n\nMore information about the error "
                                   "may be found in the terminal.");
    return false;
}

bool kdisk_save_filter_sets(const std::vector<filter_set_s*>& filterSets,
                            const QString &targetFilename)
{
    file_writer_c outFile(targetFilename);

    for (const auto *set: filterSets)
    {
        // Save the resolutions.
        {
            // Encode the filter set's activation in the resolution values, where 0
            // means the set activates for all resolutions.
            resolution_s inRes = set->inRes, outRes = set->outRes;
            if (set->activation & filter_set_s::activation_e::all)
            {
                inRes = {0, 0};
                outRes = {0, 0};
            }
            else
            {
                if (!(set->activation & filter_set_s::activation_e::in)) inRes = {0, 0};
                if (!(set->activation & filter_set_s::activation_e::out)) outRes = {0, 0};
            }

            outFile << "inout,"
                    << inRes.w << "," << inRes.h << ","
                    << outRes.w << "," << outRes.h << "\n";
        }

        outFile << "description,{" << QString::fromStdString(set->description) << "}\n";
        outFile << "enabled," << set->isEnabled << "\n";
        outFile << "scaler,{" << QString::fromStdString(set->scaler->name) << "}\n";

        // Save the filters.
        auto save_filter_data = [&](std::vector<filter_s> filters, const QString &filterType)
        {
            for (auto &filter: filters)
            {
                outFile << filterType << ",{" << QString::fromStdString(filter.uuid) << "}," << FILTER_DATA_LENGTH;
                for (uint q = 0; q < FILTER_DATA_LENGTH; q++)
                {
                    outFile << "," << (u8)filter.data[q];
                }
                outFile << "\n";
            }
        };
        outFile << "preFilters," << set->preFilters.size() << "\n";
        save_filter_data(set->preFilters, "pre");

        outFile << "postFilters," << set->postFilters.size() << "\n";
        save_filter_data(set->postFilters, "post");

        outFile << "\n";
    }

    if (!outFile.is_valid() ||
        !outFile.save_and_close())
    {
        NBENE(("Failed to write filter sets to file."));
        goto fail;
    }

    kpropagate_saved_filter_sets_to_disk(filterSets, targetFilename.toStdString());

    return true;

    fail:
    kd_show_headless_error_message("Data was not saved",
                                   "An error was encountered while preparing the filter "
                                   "sets for saving. No data was saved. \n\nMore "
                                   "information about the error may be found in the terminal.");
    return false;
}

/// TODO. Needs cleanup.
bool kdisk_load_filter_sets(const std::string &sourceFilename)
{
    std::vector<filter_set_s*> filterSets;

    if (sourceFilename.empty())
    {
        INFO(("No filter set file defined, skipping."));
        return true;
    }

    QList<QStringList> rowData = csv_parse_c(QString::fromStdString(sourceFilename)).contents();
    if (rowData.isEmpty())
    {
        goto fail;
    }

    // Each mode is saved as a block of rows, starting with a 3-element row defining
    // the mode's resolution, followed by several 2-element rows defining the various
    // video and color parameters for the resolution.
    for (int row = 0; row < rowData.count();)
    {
        filter_set_s *set = new filter_set_s;

        if ((rowData[row].count() != 5) ||
            (rowData[row].at(0) != "inout"))
        {
            NBENE(("Expected a 5-parameter 'inout' statement to begin a filter set block."));
            goto fail;
        }
        set->inRes.w = rowData[row].at(1).toUInt();
        set->inRes.h = rowData[row].at(2).toUInt();
        set->outRes.w = rowData[row].at(3).toUInt();
        set->outRes.h = rowData[row].at(4).toUInt();

        set->activation = 0;
        if (set->inRes.w == 0 && set->inRes.h == 0 &&
            set->outRes.w == 0 && set->outRes.h == 0)
        {
            set->activation |= filter_set_s::activation_e::all;
        }
        else
        {
            if (set->inRes.w != 0 && set->inRes.h != 0) set->activation |= filter_set_s::activation_e::in;
            if (set->outRes.w != 0 && set->outRes.h != 0) set->activation |= filter_set_s::activation_e::out;
        }

        #define verify_first_element_on_row_is(name) if (rowData[row].at(0) != name)\
                                                     {\
                                                        NBENE(("Error while loading filter set file: expected '%s' but got '%s'.",\
                                                               name, rowData[row].at(0).toStdString().c_str()));\
                                                        goto fail;\
                                                     }

        row++;
        if (rowData[row].at(0) != "enabled") // Legacy support, 'description' was pushed in front of 'enabled' in later revisions.
        {
            verify_first_element_on_row_is("description");
            set->description = rowData[row].at(1).toStdString();

            row++;
        }
        else
        {
            set->description = "";
        }

        verify_first_element_on_row_is("enabled");
        set->isEnabled = rowData[row].at(1).toInt();

        row++;
        verify_first_element_on_row_is("scaler");
        set->scaler = ks_scaler_for_name_string(rowData[row].at(1).toStdString());

        // Takes in a proposed UUID string and returns a valid version of it. Provides backwards-
        // compatibility with older versions of VCS, which saved filters by name rather than by
        // UUID.
        const auto uuid_string = [](const QString proposedString)->std::string
        {
            // Replace legacy VCS filter names with their corresponding UUIDs.
            if (proposedString == "Delta Histogram") return "fc85a109-c57a-4317-994f-786652231773";
            if (proposedString == "Unique Count")    return "badb0129-f48c-4253-a66f-b0ec94e225a0";
            if (proposedString == "Unsharp Mask")    return "03847778-bb9c-4e8c-96d5-0c10335c4f34";
            if (proposedString == "Blur")            return "a5426f2e-b060-48a9-adf8-1646a2d3bd41";
            if (proposedString == "Decimate")        return "eb586eb4-2d9d-41b4-9e32-5cbcf0bbbf03";
            if (proposedString == "Denoise")         return "94adffac-be42-43ac-9839-9cc53a6d615c";
            if (proposedString == "Denoise (NLM)")   return "e31d5ee3-f5df-4e7c-81b8-227fc39cbe76";
            if (proposedString == "Sharpen")         return "1c25bbb1-dbf4-4a03-93a1-adf24b311070";
            if (proposedString == "Median")          return "de60017c-afe5-4e5e-99ca-aca5756da0e8";
            if (proposedString == "Crop")            return "2448cf4a-112d-4d70-9fc1-b3e9176b6684";
            if (proposedString == "Flip")            return "80a3ac29-fcec-4ae0-ad9e-bbd8667cc680";
            if (proposedString == "Rotate")          return "140c514d-a4b0-4882-abc6-b4e9e1ff4451";

            // Otherwise, we'll assume the string is already a valid UUID.
            return proposedString.toStdString();
        };

        row++;
        verify_first_element_on_row_is("preFilters");
        const uint numPreFilters = rowData[row].at(1).toUInt();
        for (uint i = 0; i < numPreFilters; i++)
        {
            row++;
            verify_first_element_on_row_is("pre");

            filter_s filter;

            filter.uuid = uuid_string(rowData[row].at(1));
            filter.name = kf_filter_name_for_uuid(filter.uuid);

            const uint numParams = rowData[row].at(2).toUInt();
            if (numPreFilters >= FILTER_DATA_LENGTH)
            {
                NBENE(("Too many parameters specified for a filter."));
                goto fail;
            }

            for (uint p = 0; p < numParams; p++)
            {
                uint datum = rowData[row].at(3 + p).toInt();
                if (datum > 255)
                {
                    NBENE(("A filter parameter had a value outside of the range allowed (0..255)."));
                    goto fail;
                }
                filter.data[p] = datum;
            }

            set->preFilters.push_back(filter);
        }

        /// TODO. Code duplication.
        row++;
        verify_first_element_on_row_is("postFilters");
        const uint numPostFilters = rowData[row].at(1).toUInt();
        for (uint i = 0; i < numPostFilters; i++)
        {
            row++;
            verify_first_element_on_row_is("post");

            filter_s filter;

            filter.uuid = uuid_string(rowData[row].at(1));
            filter.name = kf_filter_name_for_uuid(filter.uuid);

            const uint numParams = rowData[row].at(2).toUInt();
            if (numPreFilters >= FILTER_DATA_LENGTH)
            {
                NBENE(("Too many parameters specified for a filter."));
                goto fail;
            }

            for (uint p = 0; p < numParams; p++)
            {
                uint datum = rowData[row].at(3 + p).toInt();
                if (datum > 255)
                {
                    NBENE(("A filter parameter had a value outside of the range allowed (0..255)."));
                    goto fail;
                }
                filter.data[p] = datum;
            }

            set->postFilters.push_back(filter);
        }

        #undef verify_first_element_on_row_is

        row++;

        filterSets.push_back(set);
    }

    kf_clear_filters();
    for (auto *const set: filterSets) kf_add_filter_set(set);

    kpropagate_loaded_filter_sets_from_disk(filterSets, sourceFilename);

    return true;

    fail:
    kd_show_headless_error_message("Data was not loaded",
                                   "An error was encountered while loading filter "
                                   "sets. No data was loaded.\n\nMore "
                                   "information about the error may be found in the terminal.");
    return false;
}

bool kdisk_load_aliases(const std::string &sourceFilename)
{
    if (sourceFilename.empty())
    {
        DEBUG(("No alias file defined, skipping."));
        return true;
    }

    std::vector<mode_alias_s> aliases;

    QList<QStringList> rowData = csv_parse_c(QString::fromStdString(sourceFilename)).contents();
    for (const auto &row: rowData)
    {
        if (row.count() != 4)
        {
            NBENE(("Expected a 4-parameter row in the alias file."));
            goto fail;
        }

        mode_alias_s a;
        a.from.w = row.at(0).toUInt();
        a.from.h = row.at(1).toUInt();
        a.to.w = row.at(2).toUInt();
        a.to.h = row.at(3).toUInt();

        aliases.push_back(a);
    }

    // Sort the parameters so they display more nicely in the GUI.
    std::sort(aliases.begin(), aliases.end(), [](const mode_alias_s &a, const mode_alias_s &b)
                                              { return (a.to.w * a.to.h) < (b.to.w * b.to.h); });

    kpropagate_loaded_aliases_from_disk(aliases, sourceFilename);

    // Signal a new input mode to force the program to re-evaluate the mode
    // parameters, in case one of the newly-loaded aliases applies to the
    // current mode.
    kpropagate_news_of_new_capture_video_mode();

    return true;

    fail:
    kd_show_headless_error_message("Data was not loaded",
                                   "An error was encountered while loading aliases. No data was "
                                   "loaded.\n\nMore information about the error may be found in "
                                   "the terminal.");
    return false;
}

bool kdisk_save_aliases(const std::vector<mode_alias_s> &aliases,
                        const QString &targetFilename)
{
    file_writer_c outFile(targetFilename);

    for (const auto &a: aliases)
    {
        outFile << a.from.w << "," << a.from.h << ","
                << a.to.w << "," << a.to.h << ",\n";
    }

    if (!outFile.is_valid() ||
        !outFile.save_and_close())
    {
        NBENE(("Failed to write aliases to file."));
        goto fail;
    }

    kpropagate_saved_aliases_to_disk(aliases, targetFilename.toStdString());

    return true;

    fail:
    kd_show_headless_error_message("Data was not saved",
                                   "An error was encountered while preparing the alias "
                                   "resolutions for saving. As a result, no data was saved. \n\nMore "
                                   "information about this error may be found in the terminal.");
    return false;
}

bool kdisk_save_filter_nodes(std::vector<FilterGraphNode*> &nodes,
                             const QString &targetFilename)
{
    file_writer_c outFile(targetFilename);

    outFile << "fileType,{VCS filter nodes}\n"
            << "fileVersion,a\n";

    // Save filter information.
    {
        outFile << "filterCount," << nodes.size() << "\n";

        for (FilterGraphNode *const node: nodes)
        {
            outFile << "id,{" << QString::fromStdString(kf_filter_id_for_type(node->associatedFilter->metaData.type)) << "}\n";

            outFile << "parameterData," << FILTER_DATA_LENGTH;
            for (unsigned i = 0; i < FILTER_DATA_LENGTH; i++)
            {
                outFile << QString(",%1").arg(node->associatedFilter->parameterData[i]);
            }
            outFile << "\n";
        }
    }

    // Save node information.
    {
        outFile << "nodeCount," << nodes.size() << "\n";

        for (FilterGraphNode *const node: nodes)
        {
            outFile << "scenePosition," << QString("%1,%2").arg(node->pos().x()).arg(node->pos().y()) << "\n";

            outFile << "connections,";
            if (node->output_edge())
            {
                outFile << node->output_edge()->connectedTo.size();
                for (const node_edge_s *const connection: node->output_edge()->connectedTo)
                {
                    const auto nodeIt = std::find(nodes.begin(), nodes.end(), connection->parentNode);
                    k_assert((nodeIt != nodes.end()), "Cannot find the target node of a connection.");
                    outFile << QString(",%1").arg(std::distance(nodes.begin(), nodeIt));
                }
                outFile << "\n";
            }
            else
            {
                outFile << "0\n";
            }
        }
    }

    if (!outFile.is_valid() ||
        !outFile.save_and_close())
    {
        NBENE(("Failed to write aliases to file."));
        goto fail;
    }

    return true;

    fail:
    kd_show_headless_error_message("Data was not saved",
                                   "An error was encountered while preparing filter data for saving. "
                                   "As a result, no data was saved. \n\nMore information about this "
                                   "error may be found in the terminal.");
    return false;
}

bool kdisk_load_filter_nodes(const QString &sourceFilename)
{
    std::vector<FilterGraphNode*> nodes;

    #define verify_first_element_on_row_is(name) if (rowData[row].at(0) != name)\
                                                 {\
                                                    NBENE(("Error while loading filter file: expected '%s' but got '%s'.",\
                                                           name, rowData[row].at(0).toStdString().c_str()));\
                                                    goto fail;\
                                                 }

    QList<QStringList> rowData = csv_parse_c(sourceFilename).contents();

    kd_clear_filter_graph();

    // Load the data.
    {
        unsigned row = 0;

        verify_first_element_on_row_is("fileType");
        //const QString fileType = rowData[row].at(1);

        row++;
        verify_first_element_on_row_is("fileVersion");
        //const QString fileVersion = rowData[row].at(1);

        row++;
        verify_first_element_on_row_is("filterCount");
        const unsigned numFilters = rowData[row].at(1).toUInt();

        // Load the filter data.
        for (unsigned i = 0; i < numFilters; i++)
        {
            row++;
            verify_first_element_on_row_is("id");
            const filter_type_enum_e filterType = kf_filter_type_for_id(rowData[row].at(1).toStdString());

            row++;
            verify_first_element_on_row_is("parameterData");
            const unsigned numParameters = rowData[row].at(1).toUInt();

            std::vector<u8> params;
            params.reserve(numParameters);
            for (unsigned p = 0; p < numParameters; p++)
            {
                params.push_back(rowData[row].at(2+p).toUInt());
            }

            const auto newNode = kd_add_filter_graph_node(filterType, (const u8*)params.data());
            nodes.push_back(newNode);
        }

        // Load the node data.
        row++;
        verify_first_element_on_row_is("nodeCount");
        const unsigned nodeCount = rowData[row].at(1).toUInt();

        for (unsigned i = 0; i < nodeCount; i++)
        {
            row++;
            verify_first_element_on_row_is("scenePosition");
            nodes.at(i)->setPos(QPointF(rowData[row].at(1).toDouble(), rowData[row].at(2).toDouble()));

            row++;
            verify_first_element_on_row_is("connections");
            const unsigned numConnections = rowData[row].at(1).toUInt();

            for (unsigned p = 0; p < numConnections; p++)
            {
                node_edge_s *const sourceEdge = nodes.at(i)->output_edge();
                node_edge_s *const targetEdge = nodes.at(rowData[row].at(2+p).toUInt())->input_edge();

                k_assert((sourceEdge && targetEdge), "Invalid source or target edge for connecting.");

                sourceEdge->connect_to(targetEdge);
            }
        }
    }

    #undef verify_first_element_on_row_is

    return true;

    fail:
    kd_show_headless_error_message("Data was not loaded",
                                   "An error was encountered while loading filters. No data was "
                                   "loaded.\n\nMore information about the error may be found in "
                                   "the terminal.");
    return false;
}
