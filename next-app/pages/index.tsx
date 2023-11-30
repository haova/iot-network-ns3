import { useEffect, useMemo, useState } from 'react'
import Marquee from 'react-fast-marquee'
import { Col, Row, Layout, Table, Alert } from 'antd/lib'

export default () => {
    const [dataSource, setDataSource] = useState([])
    const [isWarning, setIsWarning] = useState(false)

    const tableData = useMemo(
        () => dataSource.map((item: any) => ({ ...item, updated_at: new Date(item.updated_at).toLocaleString() })),
        [dataSource]
    )

    const ws = new WebSocket(process.env.WEBSOCKET_URL ?? '')

    useEffect(() => {
        ws.onmessage = (event) => {
            const data = JSON.parse(event.data)
            setDataSource(data.sensors.map((item: any) => item.doc))
            setIsWarning(data.isWarning)
        }
    }, [])

    const columns = [
        {
            title: 'Sensor Name',
            dataIndex: 'sensor_name',
            key: 'sensor_name',
        },
        {
            title: 'PDR',
            dataIndex: 'pdr',
            key: 'pdr',
        },
        {
            title: 'RSS',
            dataIndex: 'rss',
            key: 'rss',
        },
        {
            title: 'Last Updated',
            dataIndex: 'updated_at',
            key: 'updated_at',
        },
    ]

    return (
        <Layout style={{ position: 'fixed', top: 0, left: 0, width: '100%', height: '100%' }}>
            <Layout.Header style={{ color: 'white', fontSize: '1.5em', fontWeight: 'bold' }}>Dashboard</Layout.Header>
            <Layout.Content>
                <Alert
                    banner
                    message={
                        <Marquee pauseOnHover gradient={false}>
                            Some warning
                        </Marquee>
                    }
                    style={{ display: isWarning ? 'flex' : 'none' }}
                />
                <Row>
                    <Col span={24}>
                        <Table dataSource={tableData} columns={columns} style={{ margin: '10px' }} />
                    </Col>
                </Row>
            </Layout.Content>
        </Layout>
    )
}
