#ifndef MZML_H
#define MZML_H

#include <QDebug>
#include <QFile>
#include <QObject>
#include <QString>
#include <base64.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <ms1.h>
#include <ms2.h>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <tinyxml2.h>
#include <zlib.h>

using ms1_ptr = std::shared_ptr<Ms1>;
using ms2_ptr = std::shared_ptr<Ms2>;
class Mzml : public QObject {
    Q_OBJECT
public:
    Mzml(QObject* parent = nullptr);

public:
    void ReadMs2FromMzmls(QStringList file_names); // 读取多个mzml文件中的二级
    void ReadMs1FromCsv(QString file_name); // 从csv表格中获取一级
    void ReadMs1FromCsvFromMzmine(QString file_name); // 从mzmine输出的表格中获取一级
    void ReadMs1FromCsvFromXcms(QString file_name); // 从xcms输出的表格中获取一级
    void ReadMs2FromMzml(QString file_name); // 读取mzml文件中的二级
    std::vector<ms2_ptr> GetLocalMs2Vector(); // 获取本地的MS2向量，不复制
    std::vector<ms1_ptr> GetLocalMs1Vector(); // 获取本地的MS1向量，不复制
    void DeleteMs2LowIntensityFragment(float radio);
    void ReSet(); // 重置mzml类，就是把现有的数据删除

private:
    void ParserMs1(tinyxml2::XMLElement* spectrum_node); // 解析一级节点
    void ParserMs2(tinyxml2::XMLElement* spectrum_node); // 解析一级节点
    char GetMsLevel(tinyxml2::XMLElement* spectrum_node); // 获取该节点是属于一级节点还是二级节点
    void GetEncodeCompressionParam(tinyxml2::XMLElement* spectrum_node); // 获取数据编码的压缩的方法
    std::shared_ptr<std::vector<float>> BytesToFloat(std::string& byte_array); // 转化byte数组成float数组，智能指针
    std::shared_ptr<std::vector<double>> BytesToDouble(std::string& byte_array); // 转化byte数组成double数组，智能指针
    std::string ZlibDecompress(const std::string& str); // 解压zlib的字符串
public:
    void SortMs2VectorByPrecursorIonMz();
    void SortMs2FragmentIonMz(); // 对所有二级的碎片mz进行排序

private:
    bool m_ms2_sort_by_precuisor_ion_mz = 0;
    bool m_ms2_sort_fragment_ion_mz = 1; // 默认读取进来的ms2的碎片mz已经是经过排序的
    std::vector<ms1_ptr> m_ms1_vector;
    std::vector<ms2_ptr> m_ms2_vector;
    // 数据的编码与压缩
    const char* m_bit_type_param = "32-bit float";
    const char* m_compression_param = "no compression";

    // 发送信息
signals:
    void SendMessage(QString message);
};

#endif // MZML_H
