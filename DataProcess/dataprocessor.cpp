#include "DataProcessor.h"
#include <QNetworkDatagram>
#include <QTimer>

DataProcessor::DataProcessor(QObject *parent)
    : QObject(parent)
{
    // 打印结构体大小以便调试
    qDebug() << "Qt HybridPacket_t Size:" << sizeof(HybridPacket_t);
    // 注意：initFile() 和 Socket 的初始化现在移到了 initWorker() 中
}

void DataProcessor::initWorker() {
    if (m_udpSocket) return;

    m_udpSocket = new QUdpSocket(this);
    qDebug() << "Worker 初始化 - 对象:" << this << "Socket:" << m_udpSocket;

    // 使用共享地址模式提高成功率
    bool success = m_udpSocket->bind(QHostAddress::Any, 12345, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (success) {
        // 【关键修复】如果 bind 成功但没有处于打开状态，强制以只读方式打开设备
        if (!m_udpSocket->isOpen()) {
            m_udpSocket->open(QIODevice::ReadOnly);
        }

        qDebug() << "子线程 UDP 绑定成功，状态:" << m_udpSocket->state()
                 << "是否打开:" << m_udpSocket->isOpen();

        connect(m_udpSocket, &QUdpSocket::readyRead, this, &DataProcessor::handleReadyRead);

        m_udpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 2 * 1024 * 1024); // 1MB
    } else {
        qDebug() << "UDP 绑定失败:" << m_udpSocket->errorString();
    }
    initFile();
}

DataProcessor::~DataProcessor() {
    if (m_file.isOpen()) {
        m_stream.flush();
        m_file.close(); // 析构时确保数据写入磁盘
    }
}
void DataProcessor::initFile()
{   
    QString fileName = QString("ballbot_%1.csv")
                           .arg(QDateTime::currentDateTime().toString("yyMMdd_HHmm"));
    m_file.setFileName(fileName);
    if (m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_stream.setDevice(&m_file); // 关联设备
        m_stream << "PkgID,Timestamp,AccX,AccY,AccZ,GyroX,GyroY,GyroZ,Roll,Pitch,Yaw,M1_Out"
                    ",M2_Out,M3_Out,M1_Angle,M2_Angle,M3_Angle,CtrlDT_us\n";
    }
}


void DataProcessor::handleReadyRead() {
    // 只要有待处理数据报
    while (m_udpSocket->hasPendingDatagrams()) {
        // 预取当前数据报的大小
        qint64 size = m_udpSocket->pendingDatagramSize();
        if (size <= 0) break;

        QByteArray datagram;
        datagram.resize(static_cast<int>(size));

        // 【关键修改】使用 readDatagram 替代 readAll
        // 它直接从套接字读取，不完全依赖 QIODevice 的 isOpen 标志位状态
        m_udpSocket->readDatagram(datagram.data(), datagram.size());

        if (!datagram.isEmpty()) {
            m_buffer.append(datagram);
            // 增加缓冲区保护：防止异常数据导致内存无限增长
            if(m_buffer.size() > 1024 * 512) m_buffer.clear();
            processBuffer();
        }
    }
}

void DataProcessor::processBuffer()
{
    const int packetSize = sizeof(HybridPacket_t); // 363 字节

    while (m_buffer.size() >= packetSize) {
        // 1. 查找帧头 0xAA 0x55
        // if (uint8_t(m_buffer[0]) == 0xAA && uint8_t(m_buffer[1]) == 0x55) {
        //     // 找到了头，打印一下尾部实际收到的字节，看看是什么
        //     //qDebug() << "头匹配成功，检查尾部字节:"
        //     //          << QString::number(uint8_t(m_buffer[packetSize-2]), 16)
        //     //          << QString::number(uint8_t(m_buffer[packetSize-1]), 16);
        //     // 2. 验证帧尾 0x0D 0x0A
        //     if (uint8_t(m_buffer[packetSize - 2]) == 0x0D &&
        //         uint8_t(m_buffer[packetSize - 1]) == 0x0A) {

        //         HybridPacket_t packet;
        //         memcpy(&packet, m_buffer.constData(), packetSize);

        //         handleHybridPacket(packet);

        //         m_buffer.remove(0, packetSize);
        //     } else {
        //         // 帧尾不对，说明数据流错位，跳过此帧头
        //         // 如果进了这里，说明帧头对了，但帧尾位置不对
        //     //    qDebug() << "帧尾匹配失败！预期的 0D 0A 未找到。当前 Buffer 前 10 字节:"
        //     //             << m_buffer.left(10).toHex(' ');
        //         m_buffer.remove(0, 2);
        //     }
        // } else {
        //     // 未找到帧头，滑动寻找
        //     m_buffer.remove(0, 1);
        // }
        if (uint8_t(m_buffer[0]) == 0xAA && uint8_t(m_buffer[1]) == 0x55) {
            if (uint8_t(m_buffer[packetSize - 2]) == 0x0D && uint8_t(m_buffer[packetSize - 1]) == 0x0A) {
                HybridPacket_t packet;
                memcpy(&packet, m_buffer.constData(), packetSize);
                handleHybridPacket(packet);
                m_buffer.remove(0, packetSize);
                continue;
            }
        }
        m_buffer.remove(0, 1);
    }
}
void DataProcessor::handleHybridPacket(const HybridPacket_t &packet)
{
    // --- 1. 解析基础信息 (处理大小端) ---
    uint16_t pkgId = qFromLittleEndian<uint16_t>(packet.packet_id);
    uint32_t baseTs = qFromLittleEndian<uint32_t>(packet.base_timestamp);
    // 调试：如果还是没反应，取消下面这行的注释，看控制台是否有输出
    qDebug() << "成功解析数据包，时间戳:" << baseTs;

    // 丢包检测 (利用 PacketID)
    if (m_lastPkgId != 0 && pkgId != (uint16_t)(m_lastPkgId + 1)) {
        qDebug() << "检测到丢包! 缺失ID数量:" << (pkgId - m_lastPkgId - 1);
    }
    m_lastPkgId = pkgId;

    // --- 2. 展开 20ms 的原始采样数据 (1ms 频率) ---
    for (int i = 0; i < 20; ++i) {
        uint32_t currentTs = baseTs + i;

        // A. 原始 IMU 数据 (1ms/次)
        const RawSample_t &raw = packet.raw_data[i];
        int16_t ax = qFromLittleEndian<int16_t>(raw.accel[0]);
        int16_t ay = qFromLittleEndian<int16_t>(raw.accel[1]);
        int16_t az = qFromLittleEndian<int16_t>(raw.accel[2]);
        int16_t gx = qFromLittleEndian<int16_t>(raw.gyro[0]);
        int16_t gy = qFromLittleEndian<int16_t>(raw.gyro[1]);
        int16_t gz = qFromLittleEndian<int16_t>(raw.gyro[2]);

        // B. 控制信息 (10ms/次) -> 0-9ms 使用 ctrl_info[0], 10-19ms 使用 ctrl_info[1]
        int ctrlIdx = (i < 10) ? 0 : 1;
        const ControlStatus_t &ctrl = packet.ctrl_info[ctrlIdx];
        float roll  = ctrl.euler[0];  // float 通常不需要大小端转换
        float pitch = ctrl.euler[1];
        float yaw   = ctrl.euler[2];
        int16_t m1_out = qFromLittleEndian<int16_t>(ctrl.motor_out[0]);
        int16_t m2_out = qFromLittleEndian<int16_t>(ctrl.motor_out[1]);
        int16_t m3_out = qFromLittleEndian<int16_t>(ctrl.motor_out[2]);
        uint16_t dt_us = qFromLittleEndian<uint16_t>(ctrl.ctrl_dt);

        // C. 电机反馈信息 (5ms/次) -> i/5 确定 slot (0,1,2,3)
        int motorSlot = i / 5;
        float m1_angle = packet.motor_data[motorSlot][0].angle;
        float m2_angle = packet.motor_data[motorSlot][1].angle;
        float m3_angle = packet.motor_data[motorSlot][2].angle;

        // --- 3. 写入 CSV 记录 ---
        if (m_file.isOpen()) {
            m_stream << pkgId << "," << currentTs << ","
                      << ax << "," << ay << "," << az << ","
                      << gx << "," << gy << "," << gz << ","
                      << roll << "," << pitch << "," << yaw << ","
                      << m1_out << "," << m2_out << "," << m3_out << ","
                      << m1_angle << "," << m2_angle << "," << m3_angle << ","
                      << dt_us << "\n";
        }

        // // 取最后一组采样点 (第19组)
        // const RawSample_t &raw = packet.raw_data[19];

        // int16_t gx = qFromLittleEndian<int16_t>(raw.gyro[0]);
        // int16_t gy = qFromLittleEndian<int16_t>(raw.gyro[1]);
        // int16_t gz = qFromLittleEndian<int16_t>(raw.gyro[2]);

        // // 假设你已经在结构体中恢复了 accel 成员
        // int16_t ax = 0; // 如果结构体还没改，暂时设为0
        // int16_t ay = 0;
        // int16_t az = 0;

        // 发射匹配 6 个参数的信号
        // emit imuRawParsed(gx, gy, gz, ax, ay, az);

        // 关键修改：发射信号给 UI 线程。为了性能，可以每 20ms 发射一次最新点，或者发射整个数组
        if (i == 19) { // 每一帧只向 UI 发送一个最新的采样点，降低 UI 刷新压力
            emit dataParsed(baseTs + i, 0.0f, packet.ctrl_info[1].euler[1], 0.0f, 0);

            emit imuRawParsed(gx, gy, gz, 0, 0, 0);
        }
    }
}
void DataProcessor::sendPidPacket(int loopType, float p, float i, float d) {
    if (!m_udpSocket) return;

    CommandPacket_t cmd;
    cmd.head[0] = 0xAA;
    cmd.head[1] = 0x55;
    cmd.type = 0x01; // 0x01 代表 PID 更新指令
    cmd.loop_type = static_cast<uint8_t>(loopType);

    // 封装参数
    cmd.p = p;
    cmd.i = i;
    cmd.d = d;

    cmd.tail[0] = 0x0D;
    cmd.tail[1] = 0x0A;

    // 通过 UDP 发送字节流到机器人
    QByteArray data(reinterpret_cast<const char*>(&cmd), sizeof(CommandPacket_t));
    m_udpSocket->writeDatagram(data, QHostAddress(m_robotIp), m_robotPort);

    // 发射状态信号，以便在日志面板显示反馈
    emit statusUpdated(QString("Sent PID to Robot: P=%1, I=%2, D=%3").arg(p).arg(i).arg(d));
}

void DataProcessor::sendModePacket(int modeIdx) {
    // 逻辑类似，type 设为 0x02，其余参数可设为 0
}

