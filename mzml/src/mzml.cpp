#include "mzml.h"

using namespace std;
Mzml::Mzml(QObject* parent)
    : QObject(parent)
{
    this->m_ms1_vector = std::vector<ms1_ptr>();
    this->m_ms2_vector = std::vector<ms2_ptr>();
}

void Mzml::ReadMs2FromMzmls(QStringList file_names)
{
    // 读取
    for (auto itr = file_names.begin(); itr != file_names.end(); itr++) {
        this->ReadMs2FromMzml(*itr);
    }

    QString text;
    text += "MS2 count:" + QString::number(m_ms2_vector.size());

    // 得到最大值
    auto max_element_iter = std::max_element(m_ms2_vector.begin(), m_ms2_vector.end(), [=](ms2_ptr ms2_1_ptr, ms2_ptr ms2_2_ptr) {
        return ms2_1_ptr->GetRt() < ms2_2_ptr->GetRt();
    });

    // 如果最大值大于200，说明单位是sec，需要转换为min
    if ((*max_element_iter)->GetRt() > 200) {
        for (auto itr : m_ms2_vector) {
            itr->SetRt(itr->GetRt() / 60);
        }
    }

    emit this->SendMessage(text);
}

void Mzml::ReadMs1FromCsv(QString file_name)
{
    //    qDebug() << "Reading ms1 from" + file_name;
    // 从mzmine峰表中读取一级数据
    ReadMs1FromCsvFromMzmine(file_name);

    // 得到最大值
    auto max_element_iter = std::max_element(m_ms1_vector.begin(), m_ms1_vector.end(), [=](ms1_ptr ms1_1_ptr, ms1_ptr ms1_2_ptr) {
        return ms1_1_ptr->GetRt() < ms1_2_ptr->GetRt();
    });

    // 如果最大值大于200，说明单位是sec，需要转换为min
    if ((*max_element_iter)->GetRt() > 200) {
        for (auto itr : m_ms1_vector) {
            itr->SetRt(itr->GetRt() / 60);
        }
    }

    QString text;
    text += "MS1 count:" + QString::number(m_ms1_vector.size());
    emit this->SendMessage(text);
}

void Mzml::ReadMs1FromCsvFromMzmine(QString file_name)
{
    // 打开文件
    QFile file(file_name);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Can't open the file!";
        return;
    }

    // 读取第一行，查看列名，获取列名对应的位置
    QByteArray line = file.readLine();
    QList<QByteArray> col_names = line.split(',');

    int mz_index = 0;
    int rt_index = 0;
    int area_index = 0;
    int intensity_index = 0;

    // 获取列名
    for (int i = 0; i < col_names.size(); i++) {
        QString str(col_names[i]);

        if (str == "rt") {
            rt_index = i;
        } else if (str == "mz") {
            mz_index = i;
        } else if (str == "area") {
            area_index = i;
        } else if (str == "intensity_range:min") {
            intensity_index = i;
        }
    }

    // 读取剩余的数据
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QList<QByteArray> data = line.split(',');
        float mz = data[mz_index].toFloat();
        float rt = data[rt_index].toFloat();
        float area = data[area_index].toFloat();
        float intensity = data[intensity_index].toFloat();
        this->m_ms1_vector.emplace_back(make_shared<Ms1>(mz, intensity, rt, area)); // 把数据传入m_ms1_vector中
    }

    file.close();
}

void Mzml::ReadMs1FromCsvFromXcms(QString file_name)
{
    ifstream fp(file_name.toStdString()); // 定义声明一个ifstream对象，指定文件路径
    string line;
    getline(fp, line); // 跳过列名，第一行不做处理
    while (getline(fp, line)) { // 循环读取每行数据
        int mz_pos = line.find_first_of(","); // 找到的第一个','的位置
        float mz = stod(line.substr(0, mz_pos));
        auto left_line = line.substr(mz_pos + 1); // 取出剩余的字符串
        int intensity_pos = left_line.find_first_of(","); // 找到的第二个','的位置
        float intensity = stod(left_line.substr(0, intensity_pos));
        float rt = stof(left_line.substr(intensity_pos + 1));
        this->m_ms1_vector.emplace_back(make_shared<Ms1>(mz, intensity, rt)); // 把数据传入m_ms1_vector中
    }

    // 关闭文件
    fp.close();
}

void Mzml::ReadMs2FromMzml(QString file_name)
{
    using namespace tinyxml2;
    XMLDocument xml;
    // 载入数据,需要类型是char*
    xml.LoadFile(file_name.toStdString().c_str());

    // 根节点
    XMLElement* spectrumList_node = xml.RootElement();

    // 如果找到了spectrumList节点，则退出循环，如果没有找到，则提示错误文件
    while (true) {
        if (spectrumList_node == nullptr) {
            qDebug() << "wrong file!!!";
            return;
        } else if (std::string(spectrumList_node->Name()) == "indexedmzML") {
            spectrumList_node = spectrumList_node->FirstChildElement("mzML");
        } else if (std::string(spectrumList_node->Name()) == "mzML") {
            spectrumList_node = spectrumList_node->FirstChildElement("run");
        } else if (std::string(spectrumList_node->Name()) == "run") {
            spectrumList_node = spectrumList_node->FirstChildElement("spectrumList");
        } else if (std::string(spectrumList_node->Name()) == "spectrumList") {
            break;
        }
    }

    // 获取压缩方式和字节编码格式
    this->GetEncodeCompressionParam(spectrumList_node->FirstChildElement());

    // 遍历所有的spectrum节点，读取信息
    for (XMLElement* spectrum_node = spectrumList_node->FirstChildElement("spectrum"); spectrum_node != nullptr; spectrum_node = spectrum_node->NextSiblingElement()) {
        char ms_level = this->GetMsLevel(spectrum_node); // 获取ms_level信息
        if (ms_level == '2') {
            this->ParserMs2(spectrum_node);
        }
    }

    return;
}

std::vector<ms2_ptr> Mzml::GetLocalMs2Vector()
{
    return (this->m_ms2_vector);
}

std::vector<ms1_ptr> Mzml::GetLocalMs1Vector()
{
    return (this->m_ms1_vector);
}

void Mzml::DeleteMs2LowIntensityFragment(float radio)
{
    for (auto itr : m_ms2_vector) {
        itr->DeleteLowIntensityFragment(radio); // 删除冗余值
        itr->CalculateTotalIntensity(); // 计算这个二级的所有碎片的强度之和
    }

    qDebug() << "二级过滤了" << radio;
}

void Mzml::ReSet()
{
    vector<ms1_ptr>().swap(m_ms1_vector);
    vector<ms2_ptr>().swap(m_ms2_vector);

    m_ms2_sort_by_precuisor_ion_mz = 0;
}

void Mzml::ParserMs1(tinyxml2::XMLElement* spectrum_node)
{
    using namespace tinyxml2;

    // 获取保留时间，单位为分钟
    const char* rt_char = spectrum_node->FirstChildElement("scanList")->FirstChildElement("scan")->FirstChildElement("cvParam")->Attribute("value");
    float rt = atof(rt_char);

    // 获取mz节点和intensity节点
    XMLElement* mz_node = spectrum_node->FirstChildElement("binaryDataArrayList")->FirstChildElement();
    XMLElement* intensity_node = mz_node->NextSiblingElement();

    // base64数据转化成字节数组
    std::string mz_data = base64_decode(mz_node->FirstChildElement("binary")->GetText());
    std::string intensity_data = base64_decode(intensity_node->FirstChildElement("binary")->GetText());

    // 解压string
    if (std::string(this->m_compression_param) == "zlib compression") {
        mz_data = this->ZlibDecompress(mz_data);
        intensity_data = this->ZlibDecompress(intensity_data);
    } else if (std::string(this->m_compression_param) == "no compression") {
    }

    // 从字节数组转化为数组，存储到m_ms1_vector中
    if (std::string(this->m_bit_type_param) == "32-bit float") {
        // 智能指针
        std::shared_ptr<std::vector<float>> mz_original_data = this->BytesToFloat(mz_data);
        std::shared_ptr<std::vector<float>> intensity_original_data = this->BytesToFloat(intensity_data);
        for (unsigned int i = 0; i < mz_original_data->size(); i++) {
            auto ms1 = make_shared<Ms1>(mz_original_data->at(i), intensity_original_data->at(i), rt);
            this->m_ms1_vector.emplace_back(ms1);
        }
    } else if (std::string(this->m_bit_type_param) == "64-bit float") {
        // 智能指针
        std::shared_ptr<std::vector<double>> mz_original_data = this->BytesToDouble(mz_data);
        std::shared_ptr<std::vector<double>> intensity_original_data = this->BytesToDouble(intensity_data);
        for (unsigned int i = 0; i < mz_original_data->size(); i++) {
            auto ms1 = make_shared<Ms1>(mz_original_data->at(i), intensity_original_data->at(i), rt);
            this->m_ms1_vector.emplace_back(ms1);
        }
    }
    return;
}

void Mzml::ParserMs2(tinyxml2::XMLElement* spectrum_node)
{
    using namespace tinyxml2;

    // 获取保留时间，单位为分钟
    const char* rt_char = spectrum_node->FirstChildElement("scanList")->FirstChildElement("scan")->FirstChildElement("cvParam")->Attribute("value");
    float rt = atof(rt_char);

    // 获取前体离子信息
    float precursor_ion_mz;
    float precursor_ion_intensity = 0;
    XMLElement* precursor_ion_node = spectrum_node->FirstChildElement("precursorList");
    // 获取参数的节点，获取真实信息
    while (true) {
        if (std::string(precursor_ion_node->Name()) != "cvParam") {
            precursor_ion_node = precursor_ion_node->FirstChildElement();
        } else {
            precursor_ion_mz = atof(precursor_ion_node->Attribute("value"));
            break;
        }
    }

    // 获取mz节点和intensity节点
    XMLElement* mz_node = spectrum_node->FirstChildElement("binaryDataArrayList")->FirstChildElement();
    XMLElement* intensity_node = mz_node->NextSiblingElement();

    // base64数据转化成字节数组
    std::string mz_data = base64_decode(mz_node->FirstChildElement("binary")->GetText());
    std::string intensity_data = base64_decode(intensity_node->FirstChildElement("binary")->GetText());

    // 解压
    if (std::string(this->m_compression_param) == "zlib compression") {
        mz_data = this->ZlibDecompress(mz_data);
        intensity_data = this->ZlibDecompress(intensity_data);
    } else if (std::string(this->m_compression_param) == "no compression") {
    }

    // 从字节数组转化为数组，存储到m_ms2_vector中
    if (std::string(this->m_bit_type_param) == "32-bit float") {
        // 智能指针
        std::shared_ptr<std::vector<float>> mz_original_data = this->BytesToFloat(mz_data);
        std::shared_ptr<std::vector<float>> intensity_original_data = this->BytesToFloat(intensity_data);
        // 删除强度为0的二级碎片
        for (int i = intensity_original_data->size() - 1; i >= 0; i--) {
            if (intensity_original_data->at(i) == 0) {
                intensity_original_data->erase(intensity_original_data->begin() + i);
                mz_original_data->erase(mz_original_data->begin() + i);
            }
        }
        auto ms2 = make_shared<Ms2>(precursor_ion_mz, precursor_ion_intensity, rt, mz_original_data, intensity_original_data);
        this->m_ms2_vector.emplace_back(ms2);
    } else if (std::string(this->m_bit_type_param) == "64-bit float") {
        // 智能指针
        std::shared_ptr<std::vector<double>> mz_original_data = this->BytesToDouble(mz_data);
        std::shared_ptr<std::vector<double>> intensity_original_data = this->BytesToDouble(intensity_data);
        // 删除强度为0的二级碎片
        for (int i = intensity_original_data->size() - 1; i >= 0; i--) {
            if (intensity_original_data->at(i) == 0) {
                intensity_original_data->erase(intensity_original_data->begin() + i);
                mz_original_data->erase(mz_original_data->begin() + i);
            }
        }
        auto ms2 = make_shared<Ms2>(precursor_ion_mz, precursor_ion_intensity, rt, mz_original_data, intensity_original_data);
        this->m_ms2_vector.emplace_back(ms2);
    }
    return;
}

char Mzml::GetMsLevel(tinyxml2::XMLElement* spectrum_node)
{
    auto ms_level_node = spectrum_node->FirstChildElement(); // 找第一个子节点
    while (true) {
        if (std::string(ms_level_node->Attribute("name")) == "ms level") {
            return *(ms_level_node->Attribute("value")); // 返回ms_level
        } else {
            ms_level_node = ms_level_node->NextSiblingElement(); // 到下一个节点
            // 判断是否溢出
            if (ms_level_node == nullptr) {
                return '0';
            }
        }
    }
}

void Mzml::GetEncodeCompressionParam(tinyxml2::XMLElement* spectrum_node)
{
    using namespace tinyxml2;

    // 获取存放参数的节点
    XMLElement* parameter_node = spectrum_node->FirstChildElement("binaryDataArrayList")->FirstChildElement("binaryDataArray")->FirstChildElement();

    while (true) {
        if (parameter_node->Attribute("name") == NULL) {
            return;
        } else if ((std::string(parameter_node->Attribute("name")) == "32-bit float") || (std::string(parameter_node->Attribute("name")) == "64-bit float")) {
            this->m_bit_type_param = parameter_node->Attribute("name");
        } else if ((std::string(parameter_node->Attribute("name")) == "no compression") || (std::string(parameter_node->Attribute("name")) == "zlib compression")) {
            this->m_compression_param = parameter_node->Attribute("name");
        }
        parameter_node = parameter_node->NextSiblingElement(); // 下一个节点
    }
}

std::shared_ptr<std::vector<float>> Mzml::BytesToFloat(std::string& byte_array)
{
    std::shared_ptr<std::vector<float>> vector_float = std::make_shared<std::vector<float>>(); // 初始化为空数组
    float output;
    for (unsigned int i = 0; i < byte_array.size(); i = i + 4) {
        *((uchar*)(&output) + 3) = byte_array[i + 3];
        *((uchar*)(&output) + 2) = byte_array[i + 2];
        *((uchar*)(&output) + 1) = byte_array[i + 1];
        *((uchar*)(&output) + 0) = byte_array[i + 0];
        vector_float->emplace_back(output);
    }
    return vector_float; // 返回数组的智能指针
}

std::shared_ptr<std::vector<double>> Mzml::BytesToDouble(std::string& byte_array)
{
    std::shared_ptr<std::vector<double>> vector_double = std::make_shared<std::vector<double>>(); // 初始化为空数组
    double output;
    for (unsigned int i = 0; i < byte_array.size(); i = i + 8) {
        *((uchar*)(&output) + 7) = byte_array[i + 7];
        *((uchar*)(&output) + 6) = byte_array[i + 6];
        *((uchar*)(&output) + 5) = byte_array[i + 5];
        *((uchar*)(&output) + 4) = byte_array[i + 4];
        *((uchar*)(&output) + 3) = byte_array[i + 3];
        *((uchar*)(&output) + 2) = byte_array[i + 2];
        *((uchar*)(&output) + 1) = byte_array[i + 1];
        *((uchar*)(&output) + 0) = byte_array[i + 0];
        vector_double->emplace_back(output);
    }
    return vector_double; // 返回数组的智能指针
}

std::string Mzml::ZlibDecompress(const std::string& str)
{
    z_stream zs; // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
        throw(std::runtime_error("inflateInit failed while decompressing."));

    zs.next_in = (Bytef*)str.data();
    zs.avail_in = str.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // get the decompressed bytes blockwise using repeated calls to inflate
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);
        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer,
                zs.total_out - outstring.size());
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) { // an error occurred that was not EOF
        std::ostringstream oss;
        oss << "Exception during zlib decompression: (" << ret << ") "
            << zs.msg;
        throw(std::runtime_error(oss.str()));
    }

    return outstring;
}

void Mzml::SortMs2VectorByPrecursorIonMz()
{
    if (this->m_ms2_sort_by_precuisor_ion_mz == 0) {
        // 排序
        sort(this->m_ms2_vector.begin(), this->m_ms2_vector.end(), [](ms2_ptr lhs, ms2_ptr rhs) {
            return lhs->GetPrecuisorIonMz() < rhs->GetPrecuisorIonMz();
        });
        // 判定为成功
        this->m_ms2_sort_by_precuisor_ion_mz = 1;
    }
}

void Mzml::SortMs2FragmentIonMz()
{
    if (m_ms2_sort_fragment_ion_mz) {
        return;
    }

    // 进行排序
    for (auto itr : m_ms2_vector) {
        itr->SortFragmentIonMz();
    }
    m_ms2_sort_fragment_ion_mz = 1;

    return;
}
