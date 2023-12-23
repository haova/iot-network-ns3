import { useEffect, useMemo, useState } from 'react'
import Marquee from 'react-fast-marquee'
import { Col, Row, Layout, Table, Alert, Badge } from 'antd/lib'

export default () => {
    const [dataSource, setDataSource] = useState([])

    const roundNumber = (num: number) => Math.round((num + Number.EPSILON) * 100) / 100

    const tableData = useMemo(
        () => dataSource.map((item: any) => ({ ...item, updatedAt: new Date(item?.at).toLocaleString(), pdr: roundNumber(item?.pdr), rssi: roundNumber(item?.rssi) })),
        [dataSource]
    )

    const ws = new WebSocket(process.env.WEBSOCKET_URL ?? '')

    useEffect(() => {
        ws.onmessage = (event) => {
            const data = JSON.parse(event.data)
            setDataSource(data.readings)
        }
    }, [])

    const columns = [
        {
            title: 'Access Point',
            dataIndex: 'ap',
            key: 'ap',
        },
        {
            title: 'Sensor Name',
            dataIndex: 'name',
            key: 'name',
        },
        {
            title: 'PDR',
            dataIndex: 'pdr',
            key: 'pdr',
        },
        {
            title: 'RSSI',
            dataIndex: 'rssi',
            key: 'rssi',
        },
        {
            title: 'Last Updated',
            dataIndex: 'updatedAt',
            key: 'updatedAt',
        },
        {
            title: 'Status',
            dataIndex: 'isWarning',
            key: 'isWarning',
            render: (isWarning: boolean | undefined) => <Badge color={ isWarning === true ? 'orange' : isWarning === false ? 'green' : 'yellow' } text={ isWarning === true ? 'Possibly Jammed' : isWarning === false ? 'Active' : '-'} />
        },
    ]

    return (
        <Layout style={{ position: 'fixed', top: 0, left: 0, width: '100%', height: '100%' }}>
            <Layout.Header style={{ color: 'white', fontSize: '1.5em', fontWeight: 'bold' }}>Dashboard</Layout.Header>
            <Layout.Content>
                <Row>
                    <Col span={24}>
                        <Table dataSource={tableData} columns={columns} style={{ margin: '10px' }} />
                    </Col>
                </Row>
            </Layout.Content>
        </Layout>
    )
}
