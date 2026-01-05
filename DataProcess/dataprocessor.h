#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QTextStream>
#include <QtEndian>
#include <QVector>
#include <QUdpSocket>
#include "plot/qcustomplot.h"

#pragma pack(push, 1)
struct RawSample_t {
    int16_t accel[3];
    int16_t gyro[3];
    int16_t mag[3];
};

struct ControlStatus_t {
    float    euler[3];
    int16_t  motor_out[3];
    uint16_t ctrl_dt;
};

struct MotorFeedback_t {
    float angle;   // 4字节，绝对角度
    float speed;   // 4字节，【新增】实时转速 (deg/s)
    uint16_t status; // 2字节，状态位
};

struct HybridPacket_t {
    uint8_t         head[2];
    uint8_t         type;
    uint16_t        packet_id;
    uint32_t        base_timestamp;
    RawSample_t     raw_data[20];
    ControlStatus_t ctrl_info[2];
    MotorFeedback_t motor_data[4][3];
    uint8_t         tail[2];
};

// 定义下行指令结构体（从 PC 发往机器人）
struct CommandPacket_t {
    uint8_t  head[2];      // 帧头: 0xAA 0x55
    uint8_t  type;         // 指令类型: 0x01 PID, 0x02 Mode
    uint8_t  loop_type;    // 环路类型: 0 平衡环, 1 速度环
    float    p;            // P 参数
    float    i;            // I 参数
    float    d;            // D 参数
    uint8_t  tail[2];      // 帧尾: 0x0D 0x0A
};
#pragma pack(pop)

class DataProcessor : public QObject
{
    Q_OBJECT
public:
    // 构造函数：接收图表指针列表
    //explicit DataProcessor(QVector<QCustomPlot*> plots, QObject *parent = nullptr);
    explicit DataProcessor(QObject *parent = nullptr);
    ~DataProcessor();

signals:
    // 定义信号：将解析后的关键数据发往 UI 线程
    // void dataParsed(uint32_t timestamp, float roll, float pitch, float yaw,
    //                 int16_t m1_out,int16_t m2_out,int16_t m3_out,
    //                 float m1_speed,float m2_speed,float m3_speed,
    //                 uint16_t m1_status, uint16_t m2_status, uint16_t m3_status);

    void dataParsed(uint32_t timestamp, float roll, float pitch, float yaw,
                    int16_t gx, int16_t gy, int16_t gz,
                    int16_t ax, int16_t ay, int16_t az,
                    // int16_t mx, int16_t my, int16_t mz,
                    int16_t m1_out,int16_t m2_out,int16_t m3_out,
                    float m1_speed,float m2_speed,float m3_speed,
                    uint16_t m1_status, uint16_t m2_status, uint16_t m3_status);

    void statusUpdated(QString status);

    void finished();

public slots:
    //void onRawDataReceived(const QByteArray &data);
    void initWorker();   // 在线程启动后执行初始化
    void handleReadyRead(); // 处理 UDP 数据

    // 处理来自 ControlPanel 的 PID 更新信号
    void sendPidPacket(int loopType, float p, float i, float d);
    // 处理模式切换信号
    void sendModePacket(int modeIdx);

private:
    void processBuffer();
    void handleHybridPacket(const HybridPacket_t &packet);
    void initFile();

    QUdpSocket* m_udpSocket = nullptr;

    QByteArray m_buffer;
    QFile m_file;
    QTextStream m_stream;

    uint32_t m_lastBaseTs = 0;
    uint16_t m_lastPkgId = 0;

    // 目标机器人的网络信息
    const QString m_robotIp = "192.168.0.104"; // ESP-01S 默认 IP
    const quint16 m_robotPort = 54321;        // 机器人监听端口

};

#endif
